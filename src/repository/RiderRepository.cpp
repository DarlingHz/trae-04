#include "repository/RiderRepository.h"
#include "utils/Database.h"
#include "utils/Logger.h"
#include <sqlite3.h>
#include <chrono>

namespace repository {

int RiderRepositoryImpl::create(const model::Rider& rider) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return -1;
    }

    std::string sql = R"(
        INSERT INTO riders (name, phone, rating, registration_time) 
        VALUES (?, ?, ?, ?)
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return -1;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, rider.get_name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, rider.get_phone().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, rider.get_rating());
    
    auto registration_time = std::chrono::system_clock::to_time_t(rider.get_registration_time());
    sqlite3_bind_int64(stmt, 4, registration_time);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR << "SQL执行失败: " << sqlite3_errmsg(db); 
        sqlite3_finalize(stmt);
        return -1;
    }

    int new_id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);

    return new_id;
}

std::optional<model::Rider> RiderRepositoryImpl::get_by_id(int id) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return std::nullopt;
    }

    std::string sql = "SELECT * FROM riders WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, id);
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    // 解析结果
    int rider_id = sqlite3_column_int(stmt, 0);
    const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    const char* phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    float rating = sqlite3_column_double(stmt, 3);
    int64_t registration_time_t = sqlite3_column_int64(stmt, 4);

    auto registration_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(registration_time_t));

    model::Rider rider(rider_id, name ? name : "", phone ? phone : "", rating, registration_time);

    sqlite3_finalize(stmt);
    return rider;
}

std::vector<model::Rider> RiderRepositoryImpl::get_all() {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return {};
    }

    std::string sql = "SELECT * FROM riders";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return {};
    }

    std::vector<model::Rider> riders;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int rider_id = sqlite3_column_int(stmt, 0);
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        float rating = sqlite3_column_double(stmt, 3);
        int64_t registration_time_t = sqlite3_column_int64(stmt, 4);

        auto registration_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(registration_time_t));

        riders.emplace_back(rider_id, name ? name : "", phone ? phone : "", rating, registration_time);
    }

    sqlite3_finalize(stmt);
    return riders;
}

bool RiderRepositoryImpl::update(const model::Rider& rider) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return false;
    }

    std::string sql = R"(
        UPDATE riders SET name = ?, phone = ?, rating = ?, registration_time = ? 
        WHERE id = ?
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, rider.get_name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, rider.get_phone().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, rider.get_rating());
    
    auto registration_time = std::chrono::system_clock::to_time_t(rider.get_registration_time());
    sqlite3_bind_int64(stmt, 4, registration_time);
    
    sqlite3_bind_int(stmt, 5, rider.get_id());

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR << "SQL执行失败: " << sqlite3_errmsg(db); 
        sqlite3_finalize(stmt);
        return false;
    }

    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    return changes > 0;
}

bool RiderRepositoryImpl::remove(int id) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return false;
    }

    std::string sql = "DELETE FROM riders WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        LOG_ERROR << "SQL执行失败: " << sqlite3_errmsg(db); 
        sqlite3_finalize(stmt);
        return false;
    }

    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    return changes > 0;
}

} // namespace repository
