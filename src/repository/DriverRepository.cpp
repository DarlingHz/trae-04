#include "repository/DriverRepository.h"
#include "utils/Database.h"
#include "utils/Logger.h"
#include <sqlite3.h>
#include <chrono>

namespace repository {

int DriverRepositoryImpl::create(const model::Driver& driver) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return -1;
    }

    std::string sql = R"(
        INSERT INTO drivers (name, license_plate, car_model, capacity, status, 
                              current_x, current_y, rating, registration_time) 
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return -1;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, driver.get_name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, driver.get_license_plate().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, driver.get_car_model().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, driver.get_capacity());
    sqlite3_bind_int(stmt, 5, static_cast<int>(driver.get_status()));
    sqlite3_bind_int(stmt, 6, driver.get_current_x());
    sqlite3_bind_int(stmt, 7, driver.get_current_y());
    sqlite3_bind_double(stmt, 8, driver.get_rating());
    
    auto registration_time = std::chrono::system_clock::to_time_t(driver.get_registration_time());
    sqlite3_bind_int64(stmt, 9, registration_time);

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

std::optional<model::Driver> DriverRepositoryImpl::get_by_id(int id) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return std::nullopt;
    }

    std::string sql = "SELECT * FROM drivers WHERE id = ?";
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
    int driver_id = sqlite3_column_int(stmt, 0);
    const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    const char* license_plate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    const char* car_model = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    int capacity = sqlite3_column_int(stmt, 4);
    int status = sqlite3_column_int(stmt, 5);
    int current_x = sqlite3_column_int(stmt, 6);
    int current_y = sqlite3_column_int(stmt, 7);
    float rating = sqlite3_column_double(stmt, 8);
    int64_t registration_time_t = sqlite3_column_int64(stmt, 9);

    auto registration_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(registration_time_t));

    model::Driver driver(driver_id, name ? name : "", license_plate ? license_plate : "", 
                          car_model ? car_model : "", capacity, static_cast<model::DriverStatus>(status), 
                          current_x, current_y, rating, registration_time);

    sqlite3_finalize(stmt);
    return driver;
}

std::vector<model::Driver> DriverRepositoryImpl::get_all() {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return {};
    }

    std::string sql = "SELECT * FROM drivers";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return {};
    }

    std::vector<model::Driver> drivers;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int driver_id = sqlite3_column_int(stmt, 0);
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* license_plate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* car_model = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        int capacity = sqlite3_column_int(stmt, 4);
        int status = sqlite3_column_int(stmt, 5);
        int current_x = sqlite3_column_int(stmt, 6);
        int current_y = sqlite3_column_int(stmt, 7);
        float rating = sqlite3_column_double(stmt, 8);
        int64_t registration_time_t = sqlite3_column_int64(stmt, 9);

        auto registration_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(registration_time_t));

        drivers.emplace_back(driver_id, name ? name : "", license_plate ? license_plate : "", 
                              car_model ? car_model : "", capacity, static_cast<model::DriverStatus>(status), 
                              current_x, current_y, rating, registration_time);
    }

    sqlite3_finalize(stmt);
    return drivers;
}

std::vector<model::Driver> DriverRepositoryImpl::get_available() {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return {};
    }

    std::string sql = "SELECT * FROM drivers WHERE status = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return {};
    }

    sqlite3_bind_int(stmt, 1, static_cast<int>(model::DriverStatus::AVAILABLE));

    std::vector<model::Driver> drivers;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int driver_id = sqlite3_column_int(stmt, 0);
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* license_plate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* car_model = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        int capacity = sqlite3_column_int(stmt, 4);
        int status = sqlite3_column_int(stmt, 5);
        int current_x = sqlite3_column_int(stmt, 6);
        int current_y = sqlite3_column_int(stmt, 7);
        float rating = sqlite3_column_double(stmt, 8);
        int64_t registration_time_t = sqlite3_column_int64(stmt, 9);

        auto registration_time = std::chrono::system_clock::from_time_t(static_cast<time_t>(registration_time_t));

        drivers.emplace_back(driver_id, name ? name : "", license_plate ? license_plate : "", 
                              car_model ? car_model : "", capacity, static_cast<model::DriverStatus>(status), 
                              current_x, current_y, rating, registration_time);
    }

    sqlite3_finalize(stmt);
    return drivers;
}

bool DriverRepositoryImpl::update(const model::Driver& driver) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return false;
    }

    std::string sql = R"(
        UPDATE drivers SET name = ?, license_plate = ?, car_model = ?, capacity = ?, status = ?, 
                          current_x = ?, current_y = ?, rating = ?, registration_time = ? 
        WHERE id = ?
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, driver.get_name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, driver.get_license_plate().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, driver.get_car_model().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, driver.get_capacity());
    sqlite3_bind_int(stmt, 5, static_cast<int>(driver.get_status()));
    sqlite3_bind_int(stmt, 6, driver.get_current_x());
    sqlite3_bind_int(stmt, 7, driver.get_current_y());
    sqlite3_bind_double(stmt, 8, driver.get_rating());
    
    auto registration_time = std::chrono::system_clock::to_time_t(driver.get_registration_time());
    sqlite3_bind_int64(stmt, 9, registration_time);
    
    sqlite3_bind_int(stmt, 10, driver.get_id());

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

bool DriverRepositoryImpl::update_status(int id, model::DriverStatus status) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return false;
    }

    std::string sql = "UPDATE drivers SET status = ? WHERE id = ?";
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

bool DriverRepositoryImpl::update_location(int id, int x, int y) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return false;
    }

    std::string sql = "UPDATE drivers SET current_x = ?, current_y = ? WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL准备失败: " << sqlite3_errmsg(db); 
        return false;
    }

    sqlite3_bind_int(stmt, 1, x);
    sqlite3_bind_int(stmt, 2, y);
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

bool DriverRepositoryImpl::remove(int id) {
    sqlite3* db = utils::Database::get_instance().get_connection();
    if (!db) {
        LOG_ERROR << "数据库连接为空"; 
        return false;
    }

    std::string sql = "DELETE FROM drivers WHERE id = ?";
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
