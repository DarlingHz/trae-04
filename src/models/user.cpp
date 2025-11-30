#include "user.h"
#include "db_pool.h"
#include "../common/logger.h"
#include "../common/error.h"
#include <sstream>
#include <random>
#include <iomanip>
#include <chrono>

namespace models {

User::User() : id_(0) {
}

User::User(long long id, const std::string& username, const std::string& passwordHash)
    : id_(id), username_(username), passwordHash_(passwordHash) {
}

long long User::getId() const {
    return id_;
}

void User::setId(long long id) {
    id_ = id;
}

const std::string& User::getUsername() const {
    return username_;
}

void User::setUsername(const std::string& username) {
    username_ = username;
}

const std::string& User::getPasswordHash() const {
    return passwordHash_;
}

void User::setPasswordHash(const std::string& passwordHash) {
    passwordHash_ = passwordHash;
}

bool User::checkPassword(const std::string& password) const {
    // 简单的密码验证，实际项目中应该使用更安全的哈希算法
    return hashPassword(password) == passwordHash_;
}

std::string User::hashPassword(const std::string& password) {
    // 生成简单的哈希值，实际项目中应该使用bcrypt、Argon2等安全的哈希算法
    // 这里为了演示，我们使用一个简单的哈希算法
    std::hash<std::string> hasher;
    size_t hash = hasher(password);
    
    // 添加一些盐值增加安全性
    std::string salt = "online_judge_salt_2023";
    size_t saltedHash = hasher(std::to_string(hash) + salt);
    
    // 转换为十六进制字符串
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << saltedHash;
    
    common::g_logger.debug("Hashed password for user");
    return ss.str();
}

DatabaseUserRepository::DatabaseUserRepository() {
}

DatabaseUserRepository::~DatabaseUserRepository() {
}

bool DatabaseUserRepository::createUser(const User& user) {
    try {
        if (!g_dbPool) {
            throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Database pool not initialized");
        }
        
        // 获取数据库连接
        DBConnectionPtr conn = g_dbPool->getConnection();
        
        // 构建插入语句
        std::stringstream ss;
        ss << "INSERT INTO users (username, password_hash) VALUES ('" 
           << user.getUsername() << "', '" << user.getPasswordHash() << "')";
        
        // 执行插入
        int result = conn->executeUpdate(ss.str());
        
        // 归还连接
        g_dbPool->releaseConnection(conn);
        
        if (result > 0) {
            common::g_logger.info("User created: %s", user.getUsername().c_str());
            return true;
        } else {
            common::g_logger.error("Failed to create user: %s", user.getUsername().c_str());
            return false;
        }
    } catch (const common::AppException& e) {
        common::g_logger.error("Error creating user: %s, code: %s", e.getMessage().c_str(), e.getCodeString().c_str());
        throw;
    } catch (const std::exception& e) {
        common::g_logger.error("Error creating user: %s", e.what());
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, e.what());
    }
}

std::shared_ptr<User> DatabaseUserRepository::getUserById(long long id) {
    try {
        if (!g_dbPool) {
            throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Database pool not initialized");
        }
        
        // 获取数据库连接
        DBConnectionPtr conn = g_dbPool->getConnection();
        
        // 构建查询语句
        std::stringstream ss;
        ss << "SELECT id, username, password_hash FROM users WHERE id = " << id;
        
        // 执行查询
        bool success = conn->executeQuery(ss.str());
        
        std::shared_ptr<User> user;
        if (success && conn->hasNext()) {
            conn->next();
            user = std::make_shared<User>(
                conn->getLongLong("id"),
                conn->getString("username"),
                conn->getString("password_hash")
            );
        }
        
        // 归还连接
        g_dbPool->releaseConnection(conn);
        
        return user;
    } catch (const common::AppException& e) {
        common::g_logger.error("Error getting user by id %lld: %s, code: %s", id, e.getMessage().c_str(), e.getCodeString().c_str());
        throw;
    } catch (const std::exception& e) {
        common::g_logger.error("Error getting user by id %lld: %s", id, e.what());
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, e.what());
    }
}

std::shared_ptr<User> DatabaseUserRepository::getUserByUsername(const std::string& username) {
    try {
        if (!g_dbPool) {
            throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Database pool not initialized");
        }
        
        // 获取数据库连接
        DBConnectionPtr conn = g_dbPool->getConnection();
        
        // 构建查询语句
        std::stringstream ss;
        ss << "SELECT id, username, password_hash FROM users WHERE username = '" << username << "'";
        
        // 执行查询
        bool success = conn->executeQuery(ss.str());
        
        std::shared_ptr<User> user;
        if (success && conn->hasNext()) {
            conn->next();
            user = std::make_shared<User>(
                conn->getLongLong("id"),
                conn->getString("username"),
                conn->getString("password_hash")
            );
        }
        
        // 归还连接
        g_dbPool->releaseConnection(conn);
        
        return user;
    } catch (const common::AppException& e) {
        common::g_logger.error("Error getting user by username %s: %s, code: %s", username.c_str(), e.getMessage().c_str(), e.getCodeString().c_str());
        throw;
    } catch (const std::exception& e) {
        common::g_logger.error("Error getting user by username %s: %s", username.c_str(), e.what());
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, e.what());
    }
}

bool DatabaseUserRepository::updateUser(const User& user) {
    try {
        if (!g_dbPool) {
            throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Database pool not initialized");
        }
        
        // 获取数据库连接
        DBConnectionPtr conn = g_dbPool->getConnection();
        
        // 构建更新语句
        std::stringstream ss;
        ss << "UPDATE users SET username = '" << user.getUsername() 
           << "', password_hash = '" << user.getPasswordHash() 
           << "' WHERE id = " << user.getId();
        
        // 执行更新
        int result = conn->executeUpdate(ss.str());
        
        // 归还连接
        g_dbPool->releaseConnection(conn);
        
        if (result > 0) {
            common::g_logger.info("User updated: %s", user.getUsername().c_str());
            return true;
        } else {
            common::g_logger.error("Failed to update user: %s", user.getUsername().c_str());
            return false;
        }
    } catch (const common::AppException& e) {
        common::g_logger.error("Error updating user: %s, code: %s", e.getMessage().c_str(), e.getCodeString().c_str());
        throw;
    } catch (const std::exception& e) {
        common::g_logger.error("Error updating user: %s", e.what());
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, e.what());
    }
}

bool DatabaseUserRepository::usernameExists(const std::string& username) {
    try {
        if (!g_dbPool) {
            throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Database pool not initialized");
        }
        
        // 获取数据库连接
        DBConnectionPtr conn = g_dbPool->getConnection();
        
        // 构建查询语句
        std::stringstream ss;
        ss << "SELECT COUNT(*) as count FROM users WHERE username = '" << username << "'";
        
        // 执行查询
        bool success = conn->executeQuery(ss.str());
        
        bool exists = false;
        if (success && conn->hasNext()) {
            conn->next();
            exists = conn->getInt("count") > 0;
        }
        
        // 归还连接
        g_dbPool->releaseConnection(conn);
        
        return exists;
    } catch (const common::AppException& e) {
        common::g_logger.error("Error checking username existence: %s, code: %s", e.getMessage().c_str(), e.getCodeString().c_str());
        throw;
    } catch (const std::exception& e) {
        common::g_logger.error("Error checking username existence: %s", e.what());
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, e.what());
    }
}

// 全局用户仓库实例
std::shared_ptr<UserRepository> g_userRepository;

bool initUserRepository() {
    try {
        g_userRepository = std::make_shared<DatabaseUserRepository>();
        common::g_logger.info("User repository initialized successfully");
        return true;
    } catch (const std::exception& e) {
        common::g_logger.error("Failed to initialize user repository: %s", e.what());
        return false;
    }
}

} // namespace models
