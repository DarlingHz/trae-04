#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>

// 数据库连接类
class Database {
public:
    Database();
    ~Database();
    
    // 打开数据库连接
    bool open(const std::string& db_path);
    
    // 关闭数据库连接
    void close();
    
    // 执行查询语句（返回结果集）
    std::vector<std::map<std::string, std::string>> execute_query(const std::string& sql);
    
    // 执行非查询语句（INSERT, UPDATE, DELETE）
    bool execute_non_query(const std::string& sql);
    
    // 开始事务
    bool begin_transaction();
    
    // 提交事务
    bool commit_transaction();
    
    // 回滚事务
    bool rollback_transaction();
    
    // 获取最后插入的ID
    int64_t get_last_insert_rowid();
    
    // 获取受影响的行数
    int get_affected_rows();
    
private:
    sqlite3* db_;
    bool is_open_;
};

#endif // DATABASE_H