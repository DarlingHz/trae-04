#ifndef RIDE_SHARING_BACKEND_DATABASE_H
#define RIDE_SHARING_BACKEND_DATABASE_H

#include <sqlite3.h>
#include <string>
#include <memory>

namespace utils {

class Database {
public:
    ~Database();

    // 单例模式
    static Database& get_instance();

    // 初始化数据库
    bool init(const std::string& db_path);

    // 获取数据库连接
    sqlite3* get_connection();

    // 执行SQL语句（不返回结果）
    bool execute(const std::string& sql);

private:
    // 创建数据库表
    void create_tables();
    Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    sqlite3* db_ = nullptr;
    std::string db_path_;
};

} // namespace utils

#endif //RIDE_SHARING_BACKEND_DATABASE_H
