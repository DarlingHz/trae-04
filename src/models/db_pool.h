#ifndef DB_POOL_H
#define DB_POOL_H

#include "db_connection.h"
#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <memory>

namespace models {

// 数据库连接池类
class DBConnectionPool {
public:
    DBConnectionPool(int poolSize, const std::string& host, const std::string& port, 
                    const std::string& username, const std::string& password, 
                    const std::string& database);
    ~DBConnectionPool();
    
    // 初始化连接池
    bool initialize();
    
    // 获取连接
    DBConnectionPtr getConnection(int timeoutMs = -1);
    
    // 归还连接
    void releaseConnection(DBConnectionPtr connection);
    
    // 关闭所有连接
    void shutdown();
    
    // 获取连接池状态
    int getActiveConnections();
    int getIdleConnections();

private:
    // 创建连接
    DBConnectionPtr createConnection();
    
    // 验证连接
    bool validateConnection(DBConnectionPtr connection);

private:
    int poolSize_;
    std::string host_;
    std::string port_;
    std::string username_;
    std::string password_;
    std::string database_;
    
    std::mutex mutex_;
    std::condition_variable cv_;
    
    std::queue<DBConnectionPtr> idleConnections_;
    std::vector<DBConnectionPtr> activeConnections_;
    
    bool isInitialized_;
    bool isShutdown_;
};

// 数据库连接池指针类型
using DBConnectionPoolPtr = std::shared_ptr<DBConnectionPool>;

// 连接池单例
extern DBConnectionPoolPtr g_dbPool;

// 初始化数据库连接池
bool initDatabasePool(const std::string& host, const std::string& port, 
                      const std::string& username, const std::string& password, 
                      const std::string& database, int poolSize);

// 关闭数据库连接池
void shutdownDatabasePool();

} // namespace models

#endif // DB_POOL_H
