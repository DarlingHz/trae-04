#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <string>
#include <sqlite3.h>
#include <vector>
#include <map>

class Database {
public:
    Database();
    ~Database();

    bool open(const std::string& db_path);
    void close();
    bool isOpen() const;

    // 执行SQL语句（不返回结果）
    bool execute(const std::string& sql);

    // 执行查询语句（返回结果集）
    using ResultSet = std::vector<std::map<std::string, std::string>>;
    bool query(const std::string& sql, ResultSet& result);

    // 获取最后插入的ID
    int64_t getLastInsertId() const;

    // 开始事务
    bool beginTransaction();

    // 提交事务
    bool commitTransaction();

    // 回滚事务
    bool rollbackTransaction();

    // 获取数据库连接
    sqlite3* getDb() const;

private:
    sqlite3* db_;
    bool is_open_;
};

#endif // DATABASE_HPP
