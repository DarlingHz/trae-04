#include "service/StatsService.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"

using namespace Utils;

StatsService::StatsService(CallLogRepository& call_log_repo, ClientRepository& client_repo) 
    : call_log_repo_(call_log_repo), client_repo_(client_repo) {
}

StatsService::~StatsService() {
}

bool StatsService::getTopClientsByDailyCalls(const std::string& date, int limit, std::vector<std::pair<int64_t, uint32_t>>& top_clients) {
    if (date.empty() || limit <= 0) {
        LOG_ERROR("Invalid input parameters for getTopClientsByDailyCalls");
        return false;
    }

    if (!call_log_repo_.getTopClientsByDailyCalls(date, limit, top_clients)) {
        LOG_ERROR("Failed to get top clients by daily calls");
        return false;
    }

    return true;
}

bool StatsService::getClientStats(int64_t client_id, const std::string& start_date, const std::string& end_date, ClientStats& stats) {
    if (client_id <= 0 || start_date.empty() || end_date.empty()) {
        LOG_ERROR("Invalid input parameters for getClientStats");
        return false;
    }

    // 验证客户端是否存在
    Client client;
    if (!client_repo_.getById(client_id, client)) {
        LOG_ERROR("Client not found: " + std::to_string(client_id));
        return false;
    }

    // 获取客户端在指定时间范围内的所有调用日志
    std::vector<CallLog> call_logs;
    std::string start_time = start_date + " 00:00:00";
    std::string end_time = end_date + " 23:59:59";
    if (!call_log_repo_.getByClientId(client_id, start_time, end_time, call_logs)) {
        LOG_ERROR("Failed to get call logs for client: " + std::to_string(client_id));
        return false;
    }

    // 计算统计数据
    stats.client_id = client_id;
    stats.client_name = client.name;
    stats.total_calls = call_logs.size();
    stats.allowed_calls = 0;
    stats.rejected_calls = 0;
    stats.rejection_reasons.clear();

    for (const auto& call_log : call_logs) {
        if (call_log.allowed) {
            stats.allowed_calls++;
        } else {
            stats.rejected_calls++;
            stats.rejection_reasons[call_log.reason]++;
        }
    }

    return true;
}

bool StatsService::getApiKeyTimeline(const std::string& api_key, const std::string& granularity, std::vector<ApiKeyTimeline>& timeline) {
    if (api_key.empty() || granularity.empty()) {
        LOG_ERROR("Invalid input parameters for getApiKeyTimeline");
        return false;
    }

    // 目前只支持按小时聚合
    if (granularity != "hour") {
        LOG_ERROR("Unsupported granularity: " + granularity);
        return false;
    }

    // 获取API密钥在最近24小时内的所有调用日志
    std::vector<CallLog> call_logs;
    time_t now = getCurrentTime();
    time_t twenty_four_hours_ago = now - 24 * 60 * 60;
    std::string start_time = formatTime(twenty_four_hours_ago, "%Y-%m-%d %H:%M:%S");
    std::string end_time = formatTime(now, "%Y-%m-%d %H:%M:%S");
    if (!call_log_repo_.getByApiKey(api_key, start_time, end_time, call_logs)) {
        LOG_ERROR("Failed to get call logs for API key: " + api_key);
        return false;
    }

    // 按小时聚合调用数据
    timeline.clear();
    std::map<std::string, ApiKeyTimeline> timeline_map;

    // 初始化最近24小时的所有小时段
    for (int i = 23; i >= 0; --i) {
        time_t hour_time = now - i * 60 * 60;
        std::string time_slot = formatTime(hour_time, "%Y-%m-%d %H:00:00");
        ApiKeyTimeline entry;
        entry.time_slot = time_slot;
        entry.call_count = 0;
        entry.allowed_count = 0;
        entry.rejected_count = 0;
        timeline_map[time_slot] = entry;
    }

    // 统计每个小时段的调用数据
    for (const auto& call_log : call_logs) {
        time_t log_time = parseTime(call_log.created_at);
        std::string time_slot = formatTime(log_time, "%Y-%m-%d %H:00:00");

        auto it = timeline_map.find(time_slot);
        if (it != timeline_map.end()) {
            it->second.call_count++;
            if (call_log.allowed) {
                it->second.allowed_count++;
            } else {
                it->second.rejected_count++;
            }
        }
    }

    // 将结果转换为向量
    for (const auto& pair : timeline_map) {
        timeline.push_back(pair.second);
    }

    return true;
}
