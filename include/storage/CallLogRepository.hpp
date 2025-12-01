#ifndef CALL_LOG_REPOSITORY_HPP
#define CALL_LOG_REPOSITORY_HPP

#include <string>
#include <cstdint>
#include <vector>
#include <map>
#include "Database.hpp"

struct CallLog {
    int64_t log_id;
    int64_t client_id;
    std::string api_key;
    std::string endpoint;
    uint32_t weight;
    bool allowed;
    std::string reason;
    std::string created_at;
};

class CallLogRepository {
public:
    CallLogRepository(Database& db);
    ~CallLogRepository();

    bool createTable();
    bool insert(const CallLog& call_log);
    bool getByClientId(int64_t client_id, const std::string& start_time, const std::string& end_time, std::vector<CallLog>& call_logs);
    bool getByApiKey(const std::string& api_key, const std::string& start_time, const std::string& end_time, std::vector<CallLog>& call_logs);
    bool getTopClientsByDailyCalls(const std::string& date, int limit, std::vector<std::pair<int64_t, uint32_t>>& top_clients);

private:
    Database& db_;
};

#endif // CALL_LOG_REPOSITORY_HPP
