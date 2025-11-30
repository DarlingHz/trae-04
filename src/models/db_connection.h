#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H

#include <string>
#include <memory>

namespace models {

// 数据库连接抽象类
class DBConnection {
public:
    virtual ~DBConnection() = default;
    
    // 执行查询语句
    virtual bool executeQuery(const std::string& query) = 0;
    
    // 执行更新语句（插入、更新、删除）
    virtual int executeUpdate(const std::string& query) = 0;
    
    // 获取自动生成的ID（插入操作后）
    virtual long long getLastInsertId() = 0;
    
    // 检查是否有结果
    virtual bool hasNext() = 0;
    
    // 移动到下一行结果
    virtual bool next() = 0;
    
    // 获取字段值
    virtual std::string getString(const std::string& field) = 0;
    virtual int getInt(const std::string& field) = 0;
    virtual long long getLongLong(const std::string& field) = 0;
    virtual double getDouble(const std::string& field) = 0;
    virtual bool getBool(const std::string& field) = 0;
    
    // 关闭连接
    virtual void close() = 0;
    
    // 检查连接是否有效
    virtual bool isValid() = 0;
};

// 数据库连接指针类型
using DBConnectionPtr = std::shared_ptr<DBConnection>;

} // namespace models

#endif // DB_CONNECTION_H
