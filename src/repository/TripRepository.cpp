#include "repository/TripRepository.h"
#include "utils/Database.h"
#include "utils/Logger.h"
#include <sqlite3.h>
#include <chrono>

namespace repository {

int TripRepositoryImpl::create(const model::Trip& trip) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return -1;
    }

    std::string sql = R"(
        INSERT INTO trips (driver_id, rider_id, ride_request_id, match_time, start_time, 
                           end_time, status, fare) 
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return -1;
    }

    // 绑定参数
    sqlite3_bind_int(stmt, 1, trip.get_driver_id());
    sqlite3_bind_int(stmt, 2, trip.get_rider_id());
    sqlite3_bind_int(stmt, 3, trip.get_ride_request_id());
    
    auto match_time = std::chrono::system_clock::to_time_t(trip.get_match_time());
    sqlite3_bind_int64(stmt, 4, match_time);
    
    auto start_time = std::chrono::system_clock::to_time_t(trip.get_start_time());
    sqlite3_bind_int64(stmt, 5, start_time);
    
    auto end_time = std::chrono::system_clock::to_time_t(trip.get_end_time());
    sqlite3_bind_int64(stmt, 6, end_time);
    
    sqlite3_bind_int(stmt, 7, static_cast<int>(trip.get_status()));
    sqlite3_bind_double(stmt, 8, trip.get_fare());

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

std::optional<model::Trip> TripRepositoryImpl::get_by_id(int id) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return std::nullopt;
    }

    std::string sql = "SELECT * FROM trips WHERE id = ?";
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
    int trip_id = sqlite3_column_int(stmt, 0);
    int driver_id = sqlite3_column_int(stmt, 1);
    int rider_id = sqlite3_column_int(stmt, 2);
    int request_id = sqlite3_column_int(stmt, 3);
    int64_t match_time_t = sqlite3_column_int64(stmt, 4);
    int64_t start_time_t = sqlite3_column_int64(stmt, 5);
    int64_t end_time_t = sqlite3_column_int64(stmt, 6);
    int status = sqlite3_column_int(stmt, 7);
    float fare = sqlite3_column_double(stmt, 8);

    auto match_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(match_time_t));
    auto start_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(start_time_t));
    auto end_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(end_time_t));

    model::Trip trip(trip_id, driver_id, rider_id, request_id, match_time, start_time, 
                      end_time, static_cast<model::TripStatus>(status), fare);

    sqlite3_finalize(stmt);
    return trip;
}

std::vector<model::Trip> TripRepositoryImpl::get_all() {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return {};
    }

    std::string sql = "SELECT * FROM trips";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return {};
    }

    std::vector<model::Trip> trips;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int trip_id = sqlite3_column_int(stmt, 0);
        int driver_id = sqlite3_column_int(stmt, 1);
        int rider_id = sqlite3_column_int(stmt, 2);
        int request_id = sqlite3_column_int(stmt, 3);
        int64_t match_time_t = sqlite3_column_int64(stmt, 4);
        int64_t start_time_t = sqlite3_column_int64(stmt, 5);
        int64_t end_time_t = sqlite3_column_int64(stmt, 6);
        int status = sqlite3_column_int(stmt, 7);
        float fare = sqlite3_column_double(stmt, 8);

        auto match_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(match_time_t));
        auto start_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(start_time_t));
        auto end_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(end_time_t));

        trips.emplace_back(trip_id, driver_id, rider_id, request_id, match_time, start_time, 
                          end_time, static_cast<model::TripStatus>(status), fare);
    }

    sqlite3_finalize(stmt);
    return trips;
}

std::vector<model::Trip> TripRepositoryImpl::get_by_driver_id(int driver_id) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return {};
    }

    std::string sql = "SELECT * FROM trips WHERE driver_id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return {};
    }

    sqlite3_bind_int(stmt, 1, driver_id);

    std::vector<model::Trip> trips;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int trip_id = sqlite3_column_int(stmt, 0);
        int driver_id = sqlite3_column_int(stmt, 1);
        int rider_id = sqlite3_column_int(stmt, 2);
        int request_id = sqlite3_column_int(stmt, 3);
        int64_t match_time_t = sqlite3_column_int64(stmt, 4);
        int64_t start_time_t = sqlite3_column_int64(stmt, 5);
        int64_t end_time_t = sqlite3_column_int64(stmt, 6);
        int status = sqlite3_column_int(stmt, 7);
        float fare = sqlite3_column_double(stmt, 8);

        auto match_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(match_time_t));
        auto start_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(start_time_t));
        auto end_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(end_time_t));

        trips.emplace_back(trip_id, driver_id, rider_id, request_id, match_time, start_time, 
                          end_time, static_cast<model::TripStatus>(status), fare);
    }

    sqlite3_finalize(stmt);
    return trips;
}

std::vector<model::Trip> TripRepositoryImpl::get_by_rider_id(int rider_id) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return {};
    }

    std::string sql = "SELECT * FROM trips WHERE rider_id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return {};
    }

    sqlite3_bind_int(stmt, 1, rider_id);

    std::vector<model::Trip> trips;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int trip_id = sqlite3_column_int(stmt, 0);
        int driver_id = sqlite3_column_int(stmt, 1);
        int rider_id = sqlite3_column_int(stmt, 2);
        int request_id = sqlite3_column_int(stmt, 3);
        int64_t match_time_t = sqlite3_column_int64(stmt, 4);
        int64_t start_time_t = sqlite3_column_int64(stmt, 5);
        int64_t end_time_t = sqlite3_column_int64(stmt, 6);
        int status = sqlite3_column_int(stmt, 7);
        float fare = sqlite3_column_double(stmt, 8);

        auto match_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(match_time_t));
        auto start_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(start_time_t));
        auto end_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(end_time_t));

        trips.emplace_back(trip_id, driver_id, rider_id, request_id, match_time, start_time, 
                          end_time, static_cast<model::TripStatus>(status), fare);
    }

    sqlite3_finalize(stmt);
    return trips;
}

std::optional<model::Trip> TripRepositoryImpl::get_by_ride_request_id(int ride_request_id) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return std::nullopt;
    }

    std::string sql = "SELECT * FROM trips WHERE ride_request_id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, ride_request_id);
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    // 解析结果
    int trip_id = sqlite3_column_int(stmt, 0);
    int driver_id = sqlite3_column_int(stmt, 1);
    int rider_id = sqlite3_column_int(stmt, 2);
    int request_id = sqlite3_column_int(stmt, 3);
    int64_t match_time_t = sqlite3_column_int64(stmt, 4);
    int64_t start_time_t = sqlite3_column_int64(stmt, 5);
    int64_t end_time_t = sqlite3_column_int64(stmt, 6);
    int status = sqlite3_column_int(stmt, 7);
    float fare = sqlite3_column_double(stmt, 8);

    auto match_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(match_time_t));
    auto start_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(start_time_t));
    auto end_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(end_time_t));

    model::Trip trip(trip_id, driver_id, rider_id, ride_request_id, match_time, start_time, 
                      end_time, static_cast<model::TripStatus>(status), fare);

    sqlite3_finalize(stmt);
    return trip;
}

bool TripRepositoryImpl::update(const model::Trip& trip) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return false;
    }

    std::string sql = R"(
        UPDATE trips SET driver_id = ?, rider_id = ?, ride_request_id = ?, match_time = ?, start_time = ?, 
                           end_time = ?, status = ?, fare = ? 
        WHERE id = ?
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return false;
    }

    // 绑定参数
    sqlite3_bind_int(stmt, 1, trip.get_driver_id());
    sqlite3_bind_int(stmt, 2, trip.get_rider_id());
    sqlite3_bind_int(stmt, 3, trip.get_ride_request_id());
    
    auto match_time = std::chrono::system_clock::to_time_t(trip.get_match_time());
    sqlite3_bind_int64(stmt, 4, match_time);
    
    auto start_time = std::chrono::system_clock::to_time_t(trip.get_start_time());
    sqlite3_bind_int64(stmt, 5, start_time);
    
    auto end_time = std::chrono::system_clock::to_time_t(trip.get_end_time());
    sqlite3_bind_int64(stmt, 6, end_time);
    
    sqlite3_bind_int(stmt, 7, static_cast<int>(trip.get_status()));
    sqlite3_bind_double(stmt, 8, trip.get_fare());
    sqlite3_bind_int(stmt, 9, trip.get_id());

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

bool TripRepositoryImpl::update_status(int id, model::TripStatus status) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return false;
    }

    std::string sql = "UPDATE trips SET status = ? WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return false;
    }

    sqlite3_bind_int(stmt, 1, static_cast<int>(status));
    sqlite3_bind_int(stmt, 2, id);

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

bool TripRepositoryImpl::update_start_time(int id, const std::chrono::system_clock::time_point& start_time) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return false;
    }

    std::string sql = "UPDATE trips SET start_time = ? WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return false;
    }

    auto start_time_t = std::chrono::system_clock::to_time_t(start_time);
    sqlite3_bind_int64(stmt, 1, start_time_t);
    sqlite3_bind_int(stmt, 2, id);

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

bool TripRepositoryImpl::update_end_time_and_fare(int id, const std::chrono::system_clock::time_point& end_time, float fare) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return false;
    }

    std::string sql = "UPDATE trips SET end_time = ?, fare = ? WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return false;
    }

    auto end_time_t = std::chrono::system_clock::to_time_t(end_time);
    sqlite3_bind_int64(stmt, 1, end_time_t);
    sqlite3_bind_double(stmt, 2, fare);
    sqlite3_bind_int(stmt, 3, id);

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

bool TripRepositoryImpl::remove(int id) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return false;
    }

    std::string sql = "DELETE FROM trips WHERE id = ?";
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
