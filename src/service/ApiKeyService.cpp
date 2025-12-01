#include "service/ApiKeyService.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"

using namespace Utils;

ApiKeyService::ApiKeyService(ApiKeyRepository& api_key_repo, ClientRepository& client_repo) 
    : api_key_repo_(api_key_repo), client_repo_(client_repo) {
}

ApiKeyService::~ApiKeyService() {
}

bool ApiKeyService::createApiKey(int64_t client_id, const std::string& expired_at, ApiKey& created_api_key) {
    // 验证客户端是否存在且启用
    Client client;
    if (!client_repo_.getById(client_id, client)) {
        LOG_ERROR("Client not found: " + std::to_string(client_id));
        return false;
    }

    if (!client.is_enabled) {
        LOG_ERROR("Client is disabled: " + std::to_string(client_id));
        return false;
    }

    // 生成高熵随机API密钥（32字节，Base64编码后约43个字符）
    std::string api_key_str = base64Encode(generateRandomString(32));

    // 验证API密钥是否唯一（虽然数据库有UNIQUE约束，但提前检查可以提供更好的错误信息）
    ApiKey existing_api_key;
    if (api_key_repo_.getByKey(api_key_str, existing_api_key)) {
        LOG_ERROR("Generated API key is not unique, this should be very rare");
        return false;
    }

    // 创建API密钥对象
    ApiKey api_key;
    api_key.client_id = client_id;
    api_key.api_key = api_key_str;
    api_key.expired_at = expired_at;
    api_key.is_revoked = false;
    api_key.created_at = getCurrentTimeStr();
    api_key.updated_at = api_key.created_at;

    // 插入数据库
    if (!api_key_repo_.insert(api_key)) {
        LOG_ERROR("Failed to insert API key into database for client: " + std::to_string(client_id));
        return false;
    }

    created_api_key = api_key;
    LOG_INFO("API key created successfully for client: " + std::to_string(client_id));
    return true;
}

bool ApiKeyService::revokeApiKey(int64_t key_id) {
    // 获取现有API密钥
    ApiKey api_key;
    if (!api_key_repo_.getById(key_id, api_key)) {
        LOG_ERROR("API key not found: " + std::to_string(key_id));
        return false;
    }

    // 吊销API密钥
    if (!api_key_repo_.revoke(key_id)) {
        LOG_ERROR("Failed to revoke API key: " + std::to_string(key_id));
        return false;
    }

    LOG_INFO("API key revoked successfully: " + std::to_string(key_id));
    return true;
}

bool ApiKeyService::getApiKeyById(int64_t key_id, ApiKey& api_key) {
    if (!api_key_repo_.getById(key_id, api_key)) {
        LOG_ERROR("API key not found: " + std::to_string(key_id));
        return false;
    }

    return true;
}

bool ApiKeyService::getApiKeyByKey(const std::string& api_key_str, ApiKey& api_key) {
    if (!api_key_repo_.getByKey(api_key_str, api_key)) {
        LOG_ERROR("API key not found: " + api_key_str);
        return false;
    }

    return true;
}

bool ApiKeyService::getApiKeysByClientId(int64_t client_id, std::vector<ApiKey>& api_keys) {
    if (!api_key_repo_.getByClientId(client_id, api_keys)) {
        LOG_ERROR("Failed to get API keys for client: " + std::to_string(client_id));
        return false;
    }

    return true;
}

bool ApiKeyService::isApiKeyValid(const std::string& api_key_str, int64_t& client_id) {
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
