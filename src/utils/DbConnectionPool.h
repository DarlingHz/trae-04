#ifndef DB_CONNECTION_POOL_H
#define DB_CONNECTION_POOL_H

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <sqlite3.h>
#include <string>
#include <memory>

namespace utils {

class DbConnectionPool {
public:
    DbConnectionPool() = default;
    ~DbConnectionPool();

    // 初始化连接池
    bool init(const std::string& db_path, int pool_size);

    // 获取连接
    std::shared_ptr<sqlite3> getConnection();

    // 释放连接
    void releaseConnection(std::shared_ptr<sqlite3> conn);

private:
    // 创建单个连接
    std::shared_ptr<sqlite3> createConnection();

private:
    std::string db_path_;
    int pool_size_ = 0;
    std::vector<std::shared_ptr<sqlite3>> connections_;
    std::queue<std::shared_ptr<sqlite3>> available_connections_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool initialized_ = false;
};

// 全局数据库连接池实例
extern DbConnectionPool g_db_pool;

} // namespace utils

#endif // DB_CONNECTION_POOL_H
