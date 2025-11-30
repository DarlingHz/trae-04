#include "storage/db_manager.h"
#include "utils/logger.h"

namespace storage {

DBManager& DBManager::getInstance() {
    static DBManager instance;
    return instance;
}

DBManager::DBManager() : db_(nullptr) {
}

DBManager::~DBManager() {
    close();
}

bool DBManager::init(const std::string& dbPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (db_ != nullptr) {
        LOG_INFO("Database is already initialized");
        return true;
    }
    
    // 打开数据库连接
    int rc = sqlite3_open(dbPath.c_str(), &db_);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to open database: ", sqlite3_errmsg(db_));
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }
    
    LOG_INFO("Database opened successfully: ", dbPath);
    
    // 创建短链接表
    const char* createLinkTable = R"(
        CREATE TABLE IF NOT EXISTS short_links (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            long_url TEXT NOT NULL,
            short_code TEXT UNIQUE NOT NULL,
            custom_alias TEXT UNIQUE,
            create_time INTEGER NOT NULL,
            expire_time INTEGER NOT NULL DEFAULT 0,
            is_enabled INTEGER NOT NULL DEFAULT 1,
            visit_count INTEGER NOT NULL DEFAULT 0
        );
    )";
    
    char* errMsg = nullptr;
    rc = sqlite3_exec(db_, createLinkTable, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to create short_links table: ", errMsg);
        sqlite3_free(errMsg);
        close();
        return false;
    }
    
    // 创建访问日志表
    const char* createLogTable = R"(
        CREATE TABLE IF NOT EXISTS visit_logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            link_id INTEGER NOT NULL,
            ip TEXT NOT NULL,
            user_agent TEXT,
            visit_time INTEGER NOT NULL,
            FOREIGN KEY (link_id) REFERENCES short_links (id)
        );
    )";
    
    rc = sqlite3_exec(db_, createLogTable, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to create visit_logs table: ", errMsg);
        sqlite3_free(errMsg);
        close();
        return false;
    }
    
    LOG_INFO("Database tables created successfully");
    
    return true;
}

sqlite3* DBManager::getConnection() {
    std::lock_guard<std::mutex> lock(mutex_);
    return db_;
}

void DBManager::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
        LOG_INFO("Database connection closed");
    }
}

} // namespace storage
