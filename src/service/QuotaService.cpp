#include "service/QuotaService.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"

using namespace Utils;

QuotaService::QuotaService(ClientRepository& client_repo, ApiKeyRepository& api_key_repo, CallLogRepository& call_log_repo) 
    : client_repo_(client_repo), api_key_repo_(api_key_repo), call_log_repo_(call_log_repo) {
}

QuotaService::~QuotaService() {
}

bool QuotaService::checkQuota(const std::string& api_key, const std::string& endpoint, uint32_t weight, QuotaCheckResult& result) {
    // 初始化结果
    result.allowed = false;
    result.reason = "unknown_error";
    result.remaining_in_minute = 0;
    result.remaining_in_day = 0;
    result.retry_after_seconds = 0;

    // 验证API密钥是否有效
    int64_t client_id;
    if (!isApiKeyValid(api_key, client_id)) {
        result.reason = "invalid_api_key";
        recordCallLog(client_id, api_key, endpoint, weight, false, result.reason);
        return false;
    }

    // 获取客户端配额信息
    ClientQuota client_quota;
    if (!getClientQuota(client_id, client_quota)) {
        result.reason = "internal_error";
        recordCallLog(client_id, api_key, endpoint, weight, false, result.reason);
        return false;
    }

    // 检查每分钟配额
    time_t now = getCurrentTime();
    if (!isThisMinute(client_quota.last_minute)) {
        // 新的一分钟，重置计数器
        client_quota.minute_used = 0;
        client_quota.last_minute = now;
    }

    if (client_quota.minute_used + weight > client_quota.per_minute_quota) {
        // 超过每分钟配额
        result.reason = "quota_exceeded_per_minute";
        // 计算重试时间（到下一分钟的秒数）
        struct tm tm_now = {};
        localtime_r(&now, &tm_now);
        result.retry_after_seconds = 60 - tm_now.tm_sec;
        recordCallLog(client_id, api_key, endpoint, weight, false, result.reason);
        return false;
    }

    // 检查每日配额
    if (!isToday(client_quota.last_minute)) {
        // 新的一天，重置计数器
        client_quota.daily_used = 0;
    }

    if (client_quota.daily_used + weight > client_quota.daily_quota) {
        // 超过每日配额
        result.reason = "quota_exceeded_daily";
        // 计算重试时间（到明天的秒数）
        struct tm tm_now = {};
        localtime_r(&now, &tm_now);
        result.retry_after_seconds = (23 - tm_now.tm_hour) * 3600 + (59 - tm_now.tm_min) * 60 + (60 - tm_now.tm_sec);
        recordCallLog(client_id, api_key, endpoint, weight, false, result.reason);
        return false;
    }

    // 配额检查通过，更新计数器
    client_quota.minute_used += weight;
    client_quota.daily_used += weight;
    client_quota.last_minute = now;

    if (!updateClientQuota(client_id, client_quota)) {
        LOG_ERROR("Failed to update client quota: " + std::to_string(client_id));
        result.reason = "internal_error";
        recordCallLog(client_id, api_key, endpoint, weight, false, result.reason);
        return false;
    }

    // 设置成功结果
    result.allowed = true;
    result.reason = "ok";
    result.remaining_in_minute = client_quota.per_minute_quota - client_quota.minute_used;
    result.remaining_in_day = client_quota.daily_quota - client_quota.daily_used;
    result.retry_after_seconds = 0;

    // 记录调用日志
    recordCallLog(client_id, api_key, endpoint, weight, true, result.reason);

    return true;
}

bool QuotaService::getClientQuota(int64_t client_id, ClientQuota& client_quota) {
    std::lock_guard<std::mutex> lock(cache_mutex_);

    // 检查缓存中是否存在
    auto it = client_quota_cache_.find(client_id);
    if (it != client_quota_cache_.end()) {
        client_quota = it->second;
        return true;
    }

    // 从数据库中获取客户端信息
    Client client;
    if (!client_repo_.getById(client_id, client)) {
        LOG_ERROR("Client not found: " + std::to_string(client_id));
        return false;
    }

    // 初始化客户端配额
    client_quota.daily_quota = client.daily_quota;
    client_quota.per_minute_quota = client.per_minute_quota;
    client_quota.daily_used = 0;
    client_quota.minute_used = 0;
    client_quota.last_minute = 0;

    // 将客户端配额放入缓存
    client_quota_cache_[client_id] = client_quota;

    return true;
}

bool QuotaService::updateClientQuota(int64_t client_id, const ClientQuota& client_quota) {
    std::lock_guard<std::mutex> lock(cache_mutex_);

    // 更新缓存
    client_quota_cache_[client_id] = client_quota;

    return true;
}

bool QuotaService::recordCallLog(int64_t client_id, const std::string& api_key, const std::string& endpoint, uint32_t weight, bool allowed, const std::string& reason) {
    // 创建调用日志对象
    CallLog call_log;
    call_log.client_id = client_id;
    call_log.api_key = api_key;
    call_log.endpoint = endpoint;
    call_log.weight = weight;
    call_log.allowed = allowed;
    call_log.reason = reason;
    call_log.created_at = getCurrentTimeStr();

    // 插入数据库
    if (!call_log_repo_.insert(call_log)) {
        LOG_ERROR("Failed to insert call log for client: " + std::to_string(client_id));
        return false;
    }

    return true;
}

bool QuotaService::isApiKeyValid(const std::string& api_key_str, int64_t& client_id) {
    // 获取API密钥
    ApiKey api_key;
    if (!api_key_repo_.getByKey(api_key_str, api_key)) {
        LOG_DEBUG("API key not found: " + api_key_str);
        return false;
    }

    // 检查API密钥是否已吊销
    if (api_key.is_revoked) {
        LOG_DEBUG("API key is revoked: " + api_key_str);
        return false;
    }

    // 检查API密钥是否已过期
    if (!api_key.expired_at.empty()) {
        time_t expired_time = parseTime(api_key.expired_at);
        time_t now = getCurrentTime();
        if (expired_time < now) {
            LOG_DEBUG("API key is expired: " + api_key_str);
            return false;
        }
    }

    // 检查关联的客户端是否存在且启用
    Client client;
    if (!client_repo_.getById(api_key.client_id, client)) {
        LOG_DEBUG("Client not found for API key: " + api_key_str);
        return false;
    }

    if (!client.is_enabled) {
        LOG_DEBUG("Client is disabled for API key: " + api_key_str);
        return false;
    }

    // API密钥有效
    client_id = api_key.client_id;
    return true;
}
