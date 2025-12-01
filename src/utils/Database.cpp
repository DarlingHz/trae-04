#include "utils/Database.h"
#include <iostream>

namespace utils {

Database::Database() {
}

Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

Database& Database::get_instance() {
    static Database instance;
    return instance;
}

bool Database::init(const std::string& db_path) {
    if (db_) {
        // 已经初始化过了
        return true;
    }

    db_path_ = db_path;
    int rc = sqlite3_open(db_path.c_str(), &db_);

    if (rc != SQLITE_OK) {
        std::cerr << "无法打开数据库: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }

    // 创建表
    create_tables();

    return true;
}

sqlite3* Database::get_connection() {
    return db_;
}

bool Database::execute(const std::string& sql) {
    if (!db_) {
        std::cerr << "数据库未初始化" << std::endl;
        return false;
    }

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        std::cerr << "SQL执行错误: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

void Database::create_tables() {
    // 创建riders表
    std::string create_riders_table = R"(
        CREATE TABLE IF NOT EXISTS riders (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            phone TEXT,
            rating REAL DEFAULT 5.0,
            registration_time INTEGER NOT NULL
        );
    )";
    execute(create_riders_table);

    // 创建drivers表
    std::string create_drivers_table = R"(
        CREATE TABLE IF NOT EXISTS drivers (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            license_plate TEXT NOT NULL,
            car_model TEXT NOT NULL,
            capacity INTEGER DEFAULT 4,
            status INTEGER DEFAULT 0,
            current_x INTEGER DEFAULT 0,
            current_y INTEGER DEFAULT 0,
            rating REAL DEFAULT 5.0,
            registration_time INTEGER NOT NULL
        );
        CREATE INDEX IF NOT EXISTS idx_drivers_status ON drivers(status);
        CREATE INDEX IF NOT EXISTS idx_drivers_current ON drivers(current_x, current_y);
    )";
    execute(create_drivers_table);

    // 创建ride_requests表
    std::string create_ride_requests_table = R"(
        CREATE TABLE IF NOT EXISTS ride_requests (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            rider_id INTEGER NOT NULL,
            start_x INTEGER NOT NULL,
            start_y INTEGER NOT NULL,
            end_x INTEGER NOT NULL,
            end_y INTEGER NOT NULL,
            earliest_departure INTEGER NOT NULL,
            latest_departure INTEGER NOT NULL,
            status INTEGER DEFAULT 0,
            create_time INTEGER NOT NULL,
            FOREIGN KEY (rider_id) REFERENCES riders(id)
        );
        CREATE INDEX IF NOT EXISTS idx_ride_requests_status ON ride_requests(status);
        CREATE INDEX IF NOT EXISTS idx_ride_requests_rider ON ride_requests(rider_id);
    )";
    execute(create_ride_requests_table);

    // 创建trips表
    std::string create_trips_table = R"(
        CREATE TABLE IF NOT EXISTS trips (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            driver_id INTEGER NOT NULL,
            rider_id INTEGER NOT NULL,
            ride_request_id INTEGER NOT NULL,
            match_time INTEGER NOT NULL,
            start_time INTEGER,
            end_time INTEGER,
            status INTEGER DEFAULT 0,
            fare REAL DEFAULT 0.0,
            FOREIGN KEY (driver_id) REFERENCES drivers(id),
            FOREIGN KEY (rider_id) REFERENCES riders(id),
            FOREIGN KEY (ride_request_id) REFERENCES ride_requests(id)
        );
        CREATE INDEX IF NOT EXISTS idx_trips_status ON trips(status);
        CREATE INDEX IF NOT EXISTS idx_trips_driver ON trips(driver_id);
        CREATE INDEX IF NOT EXISTS idx_trips_rider ON trips(rider_id);
    )";
    execute(create_trips_table);
}

} // namespace utils
