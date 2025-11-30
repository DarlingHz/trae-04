#include "db_pool.h"
#include "db_connection.h"
#include "../common/logger.h"
#include "../common/error.h"
#include <chrono>
#include <thread>

namespace models {

DBConnectionPool::DBConnectionPool(int poolSize, const std::string& host, const std::string& port, 
                                 const std::string& username, const std::string& password, 
                                 const std::string& database) 
    : poolSize_(poolSize), host_(host), port_(port), username_(username), 
      password_(password), database_(database), isInitialized_(false), isShutdown_(false) {
}

DBConnectionPool::~DBConnectionPool() {
    shutdown();
}

bool DBConnectionPool::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isInitialized_) {
        return true;
    }
    
    common::g_logger.info("Initializing database connection pool with %d connections", poolSize_);
    
    // 创建连接池中的初始连接
    for (int i = 0; i < poolSize_; i++) {
        DBConnectionPtr connection = createConnection();
        if (connection && connection->isValid()) {
            idleConnections_.push(connection);
        } else {
            common::g_logger.error("Failed to create database connection %d", i);
        }
    }
    
    if (idleConnections_.empty()) {
        common::g_logger.error("Failed to initialize connection pool: no valid connections created");
        return false;
    }
    
    common::g_logger.info("Connection pool initialized successfully with %zu idle connections", idleConnections_.size());
    isInitialized_ = true;
    return true;
}

DBConnectionPtr DBConnectionPool::getConnection(int timeoutMs) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    if (!isInitialized_) {
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Connection pool not initialized");
    }
    
    if (isShutdown_) {
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Connection pool is shutdown");
    }
    
    // 等待可用连接
    if (idleConnections_.empty()) {
        if (timeoutMs < 0) {
            // 无限等待
            cv_.wait(lock, [this] { return !idleConnections_.empty() || isShutdown_; });
        } else {
            // 有限等待
            auto result = cv_.wait_for(lock, std::chrono::milliseconds(timeoutMs), 
                                     [this] { return !idleConnections_.empty() || isShutdown_; });
            
            if (!result) {
                throw common::AppException(common::ErrorCode::CONNECTION_POOL_EXHAUSTED, "No connections available in pool");
            }
        }
    }
    
    if (isShutdown_) {
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Connection pool is shutdown");
    }
    
    // 从空闲连接队列获取连接
    DBConnectionPtr connection = idleConnections_.front();
    idleConnections_.pop();
    
    // 验证连接是否有效
    if (!validateConnection(connection)) {
        common::g_logger.warning("Connection is not valid, creating new one");
        connection = createConnection();
        if (!connection || !connection->isValid()) {
            throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Failed to create valid connection");
        }
    }
    
    // 添加到活动连接列表
    activeConnections_.push_back(connection);
    
    common::g_logger.debug("Connection acquired from pool, active: %zu, idle: %zu", 
                         activeConnections_.size(), idleConnections_.size());
    
    return connection;
}

void DBConnectionPool::releaseConnection(DBConnectionPtr connection) {
    if (!connection) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isShutdown_) {
        connection->close();
        return;
    }
    
    // 从活动连接列表中移除
    auto it = std::find(activeConnections_.begin(), activeConnections_.end(), connection);
    if (it != activeConnections_.end()) {
        activeConnections_.erase(it);
    }
    
    // 检查连接是否有效，如果有效则归还到空闲队列
    if (connection->isValid()) {
        idleConnections_.push(connection);
        cv_.notify_one();
    } else {
        common::g_logger.warning("Released connection is not valid");
    }
    
    common::g_logger.debug("Connection released back to pool, active: %zu, idle: %zu", 
                         activeConnections_.size(), idleConnections_.size());
}

void DBConnectionPool::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isShutdown_) {
        return;
    }
    
    common::g_logger.info("Shutting down database connection pool");
    
    isShutdown_ = true;
    
    // 关闭所有空闲连接
    while (!idleConnections_.empty()) {
        DBConnectionPtr connection = idleConnections_.front();
        idleConnections_.pop();
        connection->close();
    }
    
    // 关闭所有活动连接
    for (auto& connection : activeConnections_) {
        connection->close();
    }
    activeConnections_.clear();
    
    isInitialized_ = false;
    cv_.notify_all();
    
    common::g_logger.info("Connection pool shutdown completed");
}

int DBConnectionPool::getActiveConnections() {
    std::lock_guard<std::mutex> lock(mutex_);
    return activeConnections_.size();
}

int DBConnectionPool::getIdleConnections() {
    std::lock_guard<std::mutex> lock(mutex_);
    return idleConnections_.size();
}

DBConnectionPtr DBConnectionPool::createConnection() {
    try {
        // 在实际项目中，这里应该根据配置创建真实的数据库连接
        // 现在我们使用内存数据库连接进行模拟
        DBConnectionPtr connection = createConnection();
        
        common::g_logger.debug("Created new database connection");
        return connection;
    } catch (const std::exception& e) {
        common::g_logger.error("Failed to create database connection: %s", e.what());
        return nullptr;
    }
}

bool DBConnectionPool::validateConnection(DBConnectionPtr connection) {
    if (!connection || !connection->isValid()) {
        return false;
    }
    
    try {
        // 简单的验证查询
        return connection->executeQuery("SELECT 1");
    } catch (...) {
        return false;
    }
}

// 全局连接池实例
DBConnectionPoolPtr g_dbPool;

bool initDatabasePool(const std::string& host, const std::string& port, 
                     const std::string& username, const std::string& password, 
                     const std::string& database, int poolSize) {
    try {
        g_dbPool = std::make_shared<DBConnectionPool>(poolSize, host, port, username, password, database);
        return g_dbPool->initialize();
    } catch (const std::exception& e) {
        common::g_logger.error("Failed to initialize database pool: %s", e.what());
        return false;
    }
}

void shutdownDatabasePool() {
    if (g_dbPool) {
        g_dbPool->shutdown();
        g_dbPool.reset();
    }
}

} // namespace models
