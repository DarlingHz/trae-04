#ifndef STATS_SERVICE_HPP
#define STATS_SERVICE_HPP

#include <string>
#include <cstdint>
#include <vector>
#include <map>
#include "storage/CallLogRepository.hpp"
#include "storage/ClientRepository.hpp"

struct ClientStats {
    int64_t client_id;
    std::string client_name;
    uint32_t total_calls;
    uint32_t allowed_calls;
    uint32_t rejected_calls;
    std::map<std::string, uint32_t> rejection_reasons;
};

struct ApiKeyTimeline {
    std::string time_slot;
    uint32_t call_count;
    uint32_t allowed_count;
    uint32_t rejected_count;
};

class StatsService {
public:
    StatsService(CallLogRepository& call_log_repo, ClientRepository& client_repo);
    ~StatsService();

    bool getTopClientsByDailyCalls(const std::string& date, int limit, std::vector<std::pair<int64_t, uint32_t>>& top_clients);
    bool getClientStats(int64_t client_id, const std::string& start_date, const std::string& end_date, ClientStats& stats);
    bool getApiKeyTimeline(const std::string& api_key, const std::string& granularity, std::vector<ApiKeyTimeline>& timeline);

private:
    CallLogRepository& call_log_repo_;
    ClientRepository& client_repo_;
};

#endif // STATS_SERVICE_HPP
