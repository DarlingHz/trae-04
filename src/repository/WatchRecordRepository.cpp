#include "WatchRecordRepository.h"
#include "../utils/DbConnectionPool.h"
#include "../utils/Logger.h"
#include "../utils/TimeUtils.h"
#include <sqlite3.h>
#include <string>
#include <sstream>
#include <optional>
#include <chrono>

namespace repository {

std::shared_ptr<model::WatchRecord> WatchRecordRepository::createWatchRecord(
    int user_id, int movie_id, const std::chrono::system_clock::time_point& start_time,
    int watch_duration, bool is_finished, std::optional<int> rating, std::optional<std::string> comment) {

    auto conn = utils::g_db_pool.getConnection();
    if (!conn) {
        LOG_ERROR("Failed to get database connection");
        return nullptr;
    }

    const char* sql = "INSERT INTO watch_records (user_id, movie_id, start_time, watch_duration, is_finished, rating, comment) VALUES (?, ?, ?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    int param_index = 1;
    rc = sqlite3_bind_int(stmt, param_index++, user_id);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 1: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    rc = sqlite3_bind_int(stmt, param_index++, movie_id);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 2: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    std::string start_time_str = utils::TimeUtils::timePointToIsoString(start_time);
    rc = sqlite3_bind_text(stmt, param_index++, start_time_str.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 3: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    rc = sqlite3_bind_int(stmt, param_index++, watch_duration);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 4: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    rc = sqlite3_bind_int(stmt, param_index++, is_finished ? 1 : 0);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 5: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    if (rating) {
        rc = sqlite3_bind_int(stmt, param_index++, *rating);
    } else {
        rc = sqlite3_bind_null(stmt, param_index++);
    }
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 6: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    if (comment) {
        rc = sqlite3_bind_text(stmt, param_index++, comment->c_str(), -1, SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, param_index++);
    }
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 7: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    int record_id = sqlite3_last_insert_rowid(conn.get());
    sqlite3_finalize(stmt);
    utils::g_db_pool.releaseConnection(conn);

    return getWatchRecordById(record_id);
}

std::shared_ptr<model::WatchRecord> WatchRecordRepository::getWatchRecordById(int id) {
    auto conn = utils::g_db_pool.getConnection();
    if (!conn) {
        LOG_ERROR("Failed to get database connection");
        return nullptr;
    }

    const char* sql = "SELECT id, user_id, movie_id, start_time, watch_duration, is_finished, rating, comment, created_at, updated_at FROM watch_records WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    rc = sqlite3_bind_int(stmt, 1, id);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    std::shared_ptr<model::WatchRecord> record = nullptr;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        int record_id = sqlite3_column_int(stmt, 0);
        int user_id = sqlite3_column_int(stmt, 1);
        int movie_id = sqlite3_column_int(stmt, 2);
        const char* start_time_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        int watch_duration = sqlite3_column_int(stmt, 4);
        bool is_finished = sqlite3_column_int(stmt, 5) != 0;
        std::optional<int> rating;
        if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) {
            rating = sqlite3_column_int(stmt, 6);
        }
        std::optional<std::string> comment;
        if (sqlite3_column_type(stmt, 7) != SQLITE_NULL) {
            comment = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        }
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));

        // 转换时间字符串为 time_point
        auto start_time = utils::TimeUtils::isoStringToTimePoint(start_time_str);
        auto created_at = utils::TimeUtils::isoStringToTimePoint(created_at_str);
        auto updated_at = utils::TimeUtils::isoStringToTimePoint(updated_at_str);

        // 转换optional类型为基础类型
        int rating_value = rating ? *rating : 0;
        std::string comment_value = comment ? *comment : "";
        
        record = std::make_shared<model::WatchRecord>(
            record_id, user_id, movie_id, start_time, watch_duration, is_finished ? 1 : 0, rating_value, comment_value, created_at, updated_at);
    } else if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
    }

    sqlite3_finalize(stmt);
    utils::g_db_pool.releaseConnection(conn);

    return record;
}

std::vector<std::shared_ptr<model::WatchRecord>> WatchRecordRepository::getWatchRecordsByUserId(
    int user_id, const std::optional<std::chrono::system_clock::time_point>& start_time,
    const std::optional<std::chrono::system_clock::time_point>& end_time, int page, int page_size) {

    auto conn = utils::g_db_pool.getConnection();
    if (!conn) {
        LOG_ERROR("Failed to get database connection");
        return {};
    }

    std::ostringstream sql; sql << "SELECT id, user_id, movie_id, start_time, watch_duration, is_finished, rating, comment, created_at, updated_at FROM watch_records WHERE user_id = ?";
    if (start_time) {
        sql << " AND start_time >= ?";
    }
    if (end_time) {
        sql << " AND start_time <= ?";
    }
    sql << " ORDER BY start_time DESC LIMIT ? OFFSET ?";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn.get(), sql.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return {};
    }

    int param_index = 1;
    rc = sqlite3_bind_int(stmt, param_index++, user_id);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind user_id parameter: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return {};
    }

    if (start_time) {
        std::string start_time_str = utils::TimeUtils::timePointToIsoString(*start_time);
        rc = sqlite3_bind_text(stmt, param_index++, start_time_str.c_str(), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) {
            LOG_ERROR("Failed to bind start_time parameter: " + std::string(sqlite3_errmsg(conn.get())));
            sqlite3_finalize(stmt);
            utils::g_db_pool.releaseConnection(conn);
            return {};
        }
    }

    if (end_time) {
        std::string end_time_str = utils::TimeUtils::timePointToIsoString(*end_time);
        rc = sqlite3_bind_text(stmt, param_index++, end_time_str.c_str(), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) {
            LOG_ERROR("Failed to bind end_time parameter: " + std::string(sqlite3_errmsg(conn.get())));
            sqlite3_finalize(stmt);
            utils::g_db_pool.releaseConnection(conn);
            return {};
        }
    }

    int offset = (page - 1) * page_size;
    rc = sqlite3_bind_int(stmt, param_index++, page_size);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind limit parameter: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return {};
    }
    rc = sqlite3_bind_int(stmt, param_index++, offset);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind offset parameter: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return {};
    }

    std::vector<std::shared_ptr<model::WatchRecord>> records;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int record_id = sqlite3_column_int(stmt, 0);
        int user_id = sqlite3_column_int(stmt, 1);
        int movie_id = sqlite3_column_int(stmt, 2);
        const char* start_time_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        int watch_duration = sqlite3_column_int(stmt, 4);
        bool is_finished = sqlite3_column_int(stmt, 5) != 0;
        std::optional<int> rating;
        if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) {
            rating = sqlite3_column_int(stmt, 6);
        }
        std::optional<std::string> comment;
        if (sqlite3_column_type(stmt, 7) != SQLITE_NULL) {
            comment = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        }
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));

        // 转换时间字符串为 time_point
        auto start_time = utils::TimeUtils::isoStringToTimePoint(start_time_str);
        auto created_at = utils::TimeUtils::isoStringToTimePoint(created_at_str);
        auto updated_at = utils::TimeUtils::isoStringToTimePoint(updated_at_str);

        // 转换optional类型为基础类型
        int rating_value = rating ? *rating : 0;
        std::string comment_value = comment ? *comment : "";
        
        auto record = std::make_shared<model::WatchRecord>(
            record_id, user_id, movie_id, start_time, watch_duration, is_finished, rating_value, comment_value, created_at, updated_at);
        records.push_back(record);
    }

    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
    }

    sqlite3_finalize(stmt);
    utils::g_db_pool.releaseConnection(conn);

    return records;
}

bool WatchRecordRepository::updateWatchRecord(const std::shared_ptr<model::WatchRecord>& record) {
    if (!record) {
        LOG_ERROR("Invalid watch record pointer");
        return false;
    }

    auto conn = utils::g_db_pool.getConnection();
    if (!conn) {
        LOG_ERROR("Failed to get database connection");
        return false;
    }

    const char* sql = "UPDATE watch_records SET start_time = ?, watch_duration = ?, is_finished = ?, rating = ?, comment = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    int param_index = 1;
    std::string start_time_str = utils::TimeUtils::timePointToIsoString(record->getStartTime());
    rc = sqlite3_bind_text(stmt, param_index++, start_time_str.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 1: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    rc = sqlite3_bind_int(stmt, param_index++, record->getWatchDuration());
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 2: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    rc = sqlite3_bind_int(stmt, param_index++, record->getIsFinished() ? 1 : 0);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 3: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    // 绑定评分
    int rating = record->getRating();
    if (rating > 0) {
        rc = sqlite3_bind_int(stmt, param_index++, rating);
    } else {
        rc = sqlite3_bind_null(stmt, param_index++);
    }
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 4: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    // 绑定评论
    const std::string& comment = record->getComment();
    if (!comment.empty()) {
        rc = sqlite3_bind_text(stmt, param_index++, comment.c_str(), -1, SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, param_index++);
    }
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 5: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    rc = sqlite3_bind_int(stmt, param_index++, record->getId());
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 6: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    int changes = sqlite3_changes(conn.get());
    sqlite3_finalize(stmt);
    utils::g_db_pool.releaseConnection(conn);

    return changes > 0;
}

bool WatchRecordRepository::deleteWatchRecord(int id) {
    auto conn = utils::g_db_pool.getConnection();
    if (!conn) {
        LOG_ERROR("Failed to get database connection");
        return false;
    }

    const char* sql = "DELETE FROM watch_records WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    rc = sqlite3_bind_int(stmt, 1, id);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    int changes = sqlite3_changes(conn.get());
    sqlite3_finalize(stmt);
    utils::g_db_pool.releaseConnection(conn);

    return changes > 0;
}

std::vector<std::shared_ptr<model::WatchRecord>> WatchRecordRepository::getAllWatchRecordsByUserId(int user_id) {
    auto conn = utils::g_db_pool.getConnection();
    if (!conn) {
        LOG_ERROR("Failed to get database connection");
        return {};
    }

    const char* sql = "SELECT id, user_id, movie_id, start_time, watch_duration, is_finished, rating, comment, created_at, updated_at FROM watch_records WHERE user_id = ? ORDER BY start_time DESC";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return {};
    }

    rc = sqlite3_bind_int(stmt, 1, user_id);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind user_id parameter: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return {};
    }

    std::vector<std::shared_ptr<model::WatchRecord>> records;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int record_id = sqlite3_column_int(stmt, 0);
        int user_id = sqlite3_column_int(stmt, 1);
        int movie_id = sqlite3_column_int(stmt, 2);
        const char* start_time_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        int watch_duration = sqlite3_column_int(stmt, 4);
        bool is_finished = sqlite3_column_int(stmt, 5) != 0;
        std::optional<int> rating;
        if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) {
            rating = sqlite3_column_int(stmt, 6);
        }
        std::optional<std::string> comment;
        if (sqlite3_column_type(stmt, 7) != SQLITE_NULL) {
            comment = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        }
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));

        // 转换时间字符串为 time_point
        auto start_time = utils::TimeUtils::isoStringToTimePoint(start_time_str);
        auto created_at = utils::TimeUtils::isoStringToTimePoint(created_at_str);
        auto updated_at = utils::TimeUtils::isoStringToTimePoint(updated_at_str);

        // 转换optional类型为基础类型
        int rating_value = rating ? *rating : 0;
        std::string comment_value = comment ? *comment : "";
        
        auto record = std::make_shared<model::WatchRecord>(
            record_id, user_id, movie_id, start_time, watch_duration, is_finished, rating_value, comment_value, created_at, updated_at);
        records.push_back(record);
    }

    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
    }

    sqlite3_finalize(stmt);
    utils::g_db_pool.releaseConnection(conn);

    return records;
}

} // namespace repository
