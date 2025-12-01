#include "storage/CallLogRepository.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"

using namespace Utils;

CallLogRepository::CallLogRepository(Database& db) : db_(db) {
}

CallLogRepository::~CallLogRepository() {
}

bool CallLogRepository::createTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS call_logs (
            log_id INTEGER PRIMARY KEY AUTOINCREMENT,
            client_id INTEGER NOT NULL,
            api_key TEXT NOT NULL,
            endpoint TEXT NOT NULL,
            weight INTEGER NOT NULL DEFAULT 1,
            allowed INTEGER NOT NULL,
            reason TEXT NOT NULL,
            created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (client_id) REFERENCES clients (client_id)
        );
    )";

    // 创建索引以提高查询性能
    std::string index_sql1 = "CREATE INDEX IF NOT EXISTS idx_call_logs_client_id ON call_logs (client_id);";
    std::string index_sql2 = "CREATE INDEX IF NOT EXISTS idx_call_logs_created_at ON call_logs (created_at);";
    std::string index_sql3 = "CREATE INDEX IF NOT EXISTS idx_call_logs_api_key ON call_logs (api_key);";

    return db_.execute(sql) && db_.execute(index_sql1) && db_.execute(index_sql2) && db_.execute(index_sql3);
}

bool CallLogRepository::insert(const CallLog& call_log) {
    std::string sql = R"(
        INSERT INTO call_logs (client_id, api_key, endpoint, weight, allowed, reason, created_at) 
        VALUES (?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_.getDb(), sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare insert call log statement: " + std::string(sqlite3_errmsg(db_.getDb())));
        return false;
    }

    sqlite3_bind_int64(stmt, 1, call_log.client_id);
    sqlite3_bind_text(stmt, 2, call_log.api_key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, call_log.endpoint.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, call_log.weight);
    sqlite3_bind_int(stmt, 5, call_log.allowed ? 1 : 0);
    sqlite3_bind_text(stmt, 6, call_log.reason.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, call_log.created_at.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to insert call log: " + std::string(sqlite3_errmsg(db_.getDb())));
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool CallLogRepository::getByClientId(int64_t client_id, const std::string& start_time, const std::string& end_time, std::vector<CallLog>& call_logs) {
    std::string sql = "SELECT * FROM call_logs WHERE client_id = ? AND created_at BETWEEN ? AND ? ORDER BY created_at DESC;";

    Database::ResultSet result;
    if (!db_.query(sql, result)) {
        LOG_ERROR("Failed to get call logs by client id");
        return false;
    }

    call_logs.clear();
    for (const auto& row : result) {
        CallLog call_log;
        call_log.log_id = std::stoll(row.at("log_id"));
        call_log.client_id = std::stoll(row.at("client_id"));
        call_log.api_key = row.at("api_key");
        call_log.endpoint = row.at("endpoint");
        call_log.weight = std::stoul(row.at("weight"));
        call_log.allowed = (row.at("allowed") == "1");
        call_log.reason = row.at("reason");
        call_log.created_at = row.at("created_at");
        call_logs.push_back(call_log);
    }

    return true;
}

bool CallLogRepository::getByApiKey(const std::string& api_key, const std::string& start_time, const std::string& end_time, std::vector<CallLog>& call_logs) {
    std::string sql = "SELECT * FROM call_logs WHERE api_key = ? AND created_at BETWEEN ? AND ? ORDER BY created_at DESC;";

    Database::ResultSet result;
    if (!db_.query(sql, result)) {
        LOG_ERROR("Failed to get call logs by api key");
        return false;
    }

    call_logs.clear();
    for (const auto& row : result) {
        CallLog call_log;
        call_log.log_id = std::stoll(row.at("log_id"));
        call_log.client_id = std::stoll(row.at("client_id"));
        call_log.api_key = row.at("api_key");
        call_log.endpoint = row.at("endpoint");
        call_log.weight = std::stoul(row.at("weight"));
        call_log.allowed = (row.at("allowed") == "1");
        call_log.reason = row.at("reason");
        call_log.created_at = row.at("created_at");
        call_logs.push_back(call_log);
    }

    return true;
}

bool CallLogRepository::getTopClientsByDailyCalls(const std::string& date, int limit, std::vector<std::pair<int64_t, uint32_t>>& top_clients) {
    std::string sql = R"(
        SELECT client_id, COUNT(*) as call_count 
        FROM call_logs 
        WHERE date(created_at) = ? 
        GROUP BY client_id 
        ORDER BY call_count DESC 
        LIMIT ?;
    )";

    Database::ResultSet result;
    if (!db_.query(sql, result)) {
        LOG_ERROR("Failed to get top clients by daily calls");
        return false;
    }

    top_clients.clear();
    for (const auto& row : result) {
        int64_t client_id = std::stoll(row.at("client_id"));
        uint32_t call_count = std::stoul(row.at("call_count"));
        top_clients.emplace_back(client_id, call_count);
    }

    return true;
}
