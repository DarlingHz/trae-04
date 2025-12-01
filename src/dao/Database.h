#pragma once

#include <sqlite3.h>
#include <string>
#include <mutex>

namespace accounting {

class Database {
public:
    ~Database();

    // 单例模式
    static Database& getInstance();

    // 打开数据库连接
    bool open(const std::string& db_path);

    // 关闭数据库连接
    void close();

    // 获取数据库连接句柄
    sqlite3* getConnection();

    // 执行 SQL 查询（返回结果集）
    sqlite3_stmt* executeQuery(const std::string& sql);

    // 执行 SQL 语句（不返回结果集）
    bool executeUpdate(const std::string& sql);

private:
    Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // 创建数据库表
    void createTables();

    sqlite3* db_ = nullptr;
    std::mutex mutex_; // 线程安全锁
};

} // namespace accounting
