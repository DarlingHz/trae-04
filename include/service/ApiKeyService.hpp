#ifndef API_KEY_SERVICE_HPP
#define API_KEY_SERVICE_HPP

#include <string>
#include <cstdint>
#include <vector>
#include "storage/ApiKeyRepository.hpp"
#include "storage/ClientRepository.hpp"

class ApiKeyService {
public:
    ApiKeyService(ApiKeyRepository& api_key_repo, ClientRepository& client_repo);
    ~ApiKeyService();

    bool createApiKey(int64_t client_id, const std::string& expired_at, ApiKey& created_api_key);
    bool revokeApiKey(int64_t key_id);
    bool getApiKeyById(int64_t key_id, ApiKey& api_key);
    bool getApiKeyByKey(const std::string& api_key_str, ApiKey& api_key);
    bool getApiKeysByClientId(int64_t client_id, std::vector<ApiKey>& api_keys);
    bool isApiKeyValid(const std::string& api_key_str, int64_t& client_id);

private:
    ApiKeyRepository& api_key_repo_;
    ClientRepository& client_repo_;
};

#endif // API_KEY_SERVICE_HPP
