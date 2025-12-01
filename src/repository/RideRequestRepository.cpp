#include "repository/RideRequestRepository.h"
#include "utils/Database.h"
#include "utils/Logger.h"
#include <sqlite3.h>
#include <chrono>

namespace repository {

int RideRequestRepositoryImpl::create(const model::RideRequest& ride_request) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return -1;
    }

    std::string sql = R"(
        INSERT INTO ride_requests (rider_id, start_x, start_y, end_x, end_y, 
                                     earliest_departure, latest_departure, status, create_time) 
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return -1;
    }

    // 绑定参数
    sqlite3_bind_int(stmt, 1, ride_request.get_rider_id());
    sqlite3_bind_int(stmt, 2, ride_request.get_start_x());
    sqlite3_bind_int(stmt, 3, ride_request.get_start_y());
    sqlite3_bind_int(stmt, 4, ride_request.get_end_x());
    sqlite3_bind_int(stmt, 5, ride_request.get_end_y());
    
    auto earliest_departure = std::chrono::system_clock::to_time_t(ride_request.get_earliest_departure());
    sqlite3_bind_int64(stmt, 6, earliest_departure);
    
    auto latest_departure = std::chrono::system_clock::to_time_t(ride_request.get_latest_departure());
    sqlite3_bind_int64(stmt, 7, latest_departure);
    
    sqlite3_bind_int(stmt, 8, static_cast<int>(ride_request.get_status()));
    
    auto create_time = std::chrono::system_clock::to_time_t(ride_request.get_create_time());
    sqlite3_bind_int64(stmt, 9, create_time);

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

std::optional<model::RideRequest> RideRequestRepositoryImpl::get_by_id(int id) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return std::nullopt;
    }

    std::string sql = "SELECT * FROM ride_requests WHERE id = ?";
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
    int request_id = sqlite3_column_int(stmt, 0);
    int rider_id = sqlite3_column_int(stmt, 1);
    int start_x = sqlite3_column_int(stmt, 2);
    int start_y = sqlite3_column_int(stmt, 3);
    int end_x = sqlite3_column_int(stmt, 4);
    int end_y = sqlite3_column_int(stmt, 5);
    int64_t earliest_departure_t = sqlite3_column_int64(stmt, 6);
    int64_t latest_departure_t = sqlite3_column_int64(stmt, 7);
    int status = sqlite3_column_int(stmt, 8);
    int64_t create_time_t = sqlite3_column_int64(stmt, 9);

    auto earliest_departure = std::chrono::system_clock::from_time_t(static_cast<time_t>(earliest_departure_t));
    auto latest_departure = std::chrono::system_clock::from_time_t(static_cast<time_t>(latest_departure_t));
    auto create_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(create_time_t));

    model::RideRequest ride_request(request_id, rider_id, start_x, start_y, end_x, end_y, 
                                      earliest_departure, latest_departure, static_cast<model::RideRequestStatus>(status), 
                                      create_time);

    sqlite3_finalize(stmt);
    return ride_request;
}

std::vector<model::RideRequest> RideRequestRepositoryImpl::get_all() {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return {};
    }

    std::string sql = "SELECT * FROM ride_requests";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return {};
    }

    std::vector<model::RideRequest> ride_requests;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int request_id = sqlite3_column_int(stmt, 0);
        int rider_id = sqlite3_column_int(stmt, 1);
        int start_x = sqlite3_column_int(stmt, 2);
        int start_y = sqlite3_column_int(stmt, 3);
        int end_x = sqlite3_column_int(stmt, 4);
        int end_y = sqlite3_column_int(stmt, 5);
        int64_t earliest_departure_t = sqlite3_column_int64(stmt, 6);
        int64_t latest_departure_t = sqlite3_column_int64(stmt, 7);
        int status = sqlite3_column_int(stmt, 8);
        int64_t create_time_t = sqlite3_column_int64(stmt, 9);

        auto earliest_departure = std::chrono::system_clock::from_time_t(static_cast<time_t>(earliest_departure_t));
        auto latest_departure = std::chrono::system_clock::from_time_t(static_cast<time_t>(latest_departure_t));
        auto create_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(create_time_t));

        ride_requests.emplace_back(request_id, rider_id, start_x, start_y, end_x, end_y, 
                                      earliest_departure, latest_departure, static_cast<model::RideRequestStatus>(status), 
                                      create_time);
    }

    sqlite3_finalize(stmt);
    return ride_requests;
}

std::vector<model::RideRequest> RideRequestRepositoryImpl::get_pending() {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return {};
    }

    std::string sql = "SELECT * FROM ride_requests WHERE status = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return {};
    }

    sqlite3_bind_int(stmt, 1, static_cast<int>(model::RideRequestStatus::PENDING));

    std::vector<model::RideRequest> ride_requests;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int request_id = sqlite3_column_int(stmt, 0);
        int rider_id = sqlite3_column_int(stmt, 1);
        int start_x = sqlite3_column_int(stmt, 2);
        int start_y = sqlite3_column_int(stmt, 3);
        int end_x = sqlite3_column_int(stmt, 4);
        int end_y = sqlite3_column_int(stmt, 5);
        int64_t earliest_departure_t = sqlite3_column_int64(stmt, 6);
        int64_t latest_departure_t = sqlite3_column_int64(stmt, 7);
        int status = sqlite3_column_int(stmt, 8);
        int64_t create_time_t = sqlite3_column_int64(stmt, 9);

        auto earliest_departure = std::chrono::system_clock::from_time_t(static_cast<time_t>(earliest_departure_t));
        auto latest_departure = std::chrono::system_clock::from_time_t(static_cast<time_t>(latest_departure_t));
        auto create_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(create_time_t));

        ride_requests.emplace_back(request_id, rider_id, start_x, start_y, end_x, end_y, 
                                      earliest_departure, latest_departure, static_cast<model::RideRequestStatus>(status), 
                                      create_time);
    }

    sqlite3_finalize(stmt);
    return ride_requests;
}

std::vector<model::RideRequest> RideRequestRepositoryImpl::get_by_rider_id(int rider_id) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return {};
    }

    std::string sql = "SELECT * FROM ride_requests WHERE rider_id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return {};
    }

    sqlite3_bind_int(stmt, 1, rider_id);

    std::vector<model::RideRequest> ride_requests;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int request_id = sqlite3_column_int(stmt, 0);
        int rider_id = sqlite3_column_int(stmt, 1);
        int start_x = sqlite3_column_int(stmt, 2);
        int start_y = sqlite3_column_int(stmt, 3);
        int end_x = sqlite3_column_int(stmt, 4);
        int end_y = sqlite3_column_int(stmt, 5);
        int64_t earliest_departure_t = sqlite3_column_int64(stmt, 6);
        int64_t latest_departure_t = sqlite3_column_int64(stmt, 7);
        int status = sqlite3_column_int(stmt, 8);
        int64_t create_time_t = sqlite3_column_int64(stmt, 9);

        auto earliest_departure = std::chrono::system_clock::from_time_t(static_cast<time_t>(earliest_departure_t));
        auto latest_departure = std::chrono::system_clock::from_time_t(static_cast<time_t>(latest_departure_t));
        auto create_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(create_time_t));

        ride_requests.emplace_back(request_id, rider_id, start_x, start_y, end_x, end_y, 
                                      earliest_departure, latest_departure, static_cast<model::RideRequestStatus>(status), 
                                      create_time);
    }

    sqlite3_finalize(stmt);
    return ride_requests;
}

bool RideRequestRepositoryImpl::update(const model::RideRequest& ride_request) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return false;
    }

    std::string sql = R"(
        UPDATE ride_requests SET rider_id = ?, start_x = ?, start_y = ?, end_x = ?, end_y = ?, 
                                 earliest_departure = ?, latest_departure = ?, status = ?, create_time = ? 
        WHERE id = ?
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return false;
    }

    // 绑定参数
    sqlite3_bind_int(stmt, 1, ride_request.get_rider_id());
    sqlite3_bind_int(stmt, 2, ride_request.get_start_x());
    sqlite3_bind_int(stmt, 3, ride_request.get_start_y());
    sqlite3_bind_int(stmt, 4, ride_request.get_end_x());
    sqlite3_bind_int(stmt, 5, ride_request.get_end_y());
    
    auto earliest_departure = std::chrono::system_clock::to_time_t(ride_request.get_earliest_departure());
    sqlite3_bind_int64(stmt, 6, earliest_departure);
    
    auto latest_departure = std::chrono::system_clock::to_time_t(ride_request.get_latest_departure());
    sqlite3_bind_int64(stmt, 7, latest_departure);
    
    sqlite3_bind_int(stmt, 8, static_cast<int>(ride_request.get_status()));
    
    auto create_time = std::chrono::system_clock::to_time_t(ride_request.get_create_time());
    sqlite3_bind_int64(stmt, 9, create_time);
    
    sqlite3_bind_int(stmt, 10, ride_request.get_id());

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

bool RideRequestRepositoryImpl::update_status(int id, model::RideRequestStatus status) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return false;
    }

    std::string sql = "UPDATE ride_requests SET status = ? WHERE id = ?";
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

bool RideRequestRepositoryImpl::remove(int id) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return false;
    }

    std::string sql = "DELETE FROM ride_requests WHERE id = ?";
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
