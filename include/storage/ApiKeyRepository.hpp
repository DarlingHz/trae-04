#ifndef API_KEY_REPOSITORY_HPP
#define API_KEY_REPOSITORY_HPP

#include <string>
#include <cstdint>
#include <vector>
#include <map>
#include "Database.hpp"

struct ApiKey {
    int64_t key_id;
    int64_t client_id;
    std::string api_key;
    std::string expired_at;
    bool is_revoked;
    std::string created_at;
    std::string updated_at;
};

class ApiKeyRepository {
public:
    ApiKeyRepository(Database& db);
    ~ApiKeyRepository();

    bool createTable();
    bool insert(ApiKey& api_key);
    bool update(const ApiKey& api_key);
    bool revoke(int64_t key_id);
    bool getById(int64_t key_id, ApiKey& api_key);
    bool getByKey(const std::string& api_key, ApiKey& api_key_obj);
    bool getByClientId(int64_t client_id, std::vector<ApiKey>& api_keys);

private:
    Database& db_;
};

#endif // API_KEY_REPOSITORY_HPP
