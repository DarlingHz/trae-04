#ifndef QUOTA_SERVICE_HPP
#define QUOTA_SERVICE_HPP

#include <string>
#include <cstdint>
#include <map>
#include <mutex>
#include "storage/ClientRepository.hpp"
#include "storage/ApiKeyRepository.hpp"
#include "storage/CallLogRepository.hpp"

struct QuotaCheckResult {
    bool allowed;
    std::string reason;
    uint32_t remaining_in_minute;
    uint32_t remaining_in_day;
    uint32_t retry_after_seconds;
};

class QuotaService {
public:
    QuotaService(ClientRepository& client_repo, ApiKeyRepository& api_key_repo, CallLogRepository& call_log_repo);
    ~QuotaService();

    bool checkQuota(const std::string& api_key, const std::string& endpoint, uint32_t weight, QuotaCheckResult& result);

private:
    struct ClientQuota {
        uint32_t daily_quota;
        uint32_t per_minute_quota;
        uint32_t daily_used;
        uint32_t minute_used;
        time_t last_minute;
    };

    bool isApiKeyValid(const std::string& api_key_str, int64_t& client_id);
    bool getClientQuota(int64_t client_id, ClientQuota& client_quota);
    bool updateClientQuota(int64_t client_id, const ClientQuota& client_quota);
    bool recordCallLog(int64_t client_id, const std::string& api_key, const std::string& endpoint, uint32_t weight, bool allowed, const std::string& reason);

    ClientRepository& client_repo_;
    ApiKeyRepository& api_key_repo_;
    CallLogRepository& call_log_repo_;

    std::map<int64_t, ClientQuota> client_quota_cache_;
    std::mutex cache_mutex_;
};

#endif // QUOTA_SERVICE_HPP
