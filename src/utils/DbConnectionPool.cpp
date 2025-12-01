#include "DbConnectionPool.h"
#include "Logger.h"
#include <iostream>

namespace utils {

DbConnectionPool g_db_pool;

DbConnectionPool::~DbConnectionPool() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& conn : connections_) {
        if (conn) {
            sqlite3_close(conn.get());
        }
    }
    connections_.clear();
    while (!available_connections_.empty()) {
        available_connections_.pop();
    }
}

bool DbConnectionPool::init(const std::string& db_path, int pool_size) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (initialized_) {
        LOG_WARN("DbConnectionPool already initialized");
        return true;
    }

    db_path_ = db_path;
    pool_size_ = pool_size;

    // 创建连接
    for (int i = 0; i < pool_size_; ++i) {
        auto conn = createConnection();
        if (!conn) {
            LOG_ERROR("Failed to create database connection");
            // 清理已创建的连接
            for (auto& c : connections_) {
                if (c) {
                    sqlite3_close(c.get());
                }
            }
            connections_.clear();
            return false;
        }
        connections_.push_back(conn);
        available_connections_.push(conn);
    }

    initialized_ = true;
    LOG_INFO("DbConnectionPool initialized with " + std::to_string(pool_size_) + " connections");
    return true;
}

std::shared_ptr<sqlite3> DbConnectionPool::getConnection() {
    std::unique_lock<std::mutex> lock(mutex_);

    if (!initialized_) {
        LOG_ERROR("DbConnectionPool not initialized");
        return nullptr;
    }

    // 等待可用连接
    cv_.wait(lock, [this]() {
        return !available_connections_.empty();
    });

    auto conn = available_connections_.front();
    available_connections_.pop();

    return conn;
}

void DbConnectionPool::releaseConnection(std::shared_ptr<sqlite3> conn) {
    if (!conn) {
        LOG_WARN("Trying to release a null database connection");
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        LOG_ERROR("DbConnectionPool not initialized");
        sqlite3_close(conn.get());
        return;
    }

    available_connections_.push(conn);
    cv_.notify_one();
}

std::shared_ptr<sqlite3> DbConnectionPool::createConnection() {
    sqlite3* db = nullptr;
    int rc = sqlite3_open(db_path_.c_str(), &db);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to open database: " + std::string(sqlite3_errmsg(db)));
        if (db) {
            sqlite3_close(db);
        }
        return nullptr;
    }

    // 设置 SQLite 配置
    sqlite3_busy_timeout(db, 5000); // 5 秒超时

    // 使用自定义删除器的 shared_ptr
    return std::shared_ptr<sqlite3>(db, [](sqlite3* d) {
        sqlite3_close(d);
    });
}

} // namespace utils
