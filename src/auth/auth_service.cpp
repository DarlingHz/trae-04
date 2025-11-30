#include "auth_service.h"
#include "../common/logger.h"
#include "../common/config.h"
#include "../models/user.h"
#include <unordered_map>
#include <cctype>

namespace auth {

AuthService::AuthService() {
    // 从配置中获取JWT密钥
    std::string jwtSecret = common::g_config.getString("jwt.secret", "default_secret_key_for_development_only");
    m_jwt = std::make_unique<JWT>(jwtSecret);
}

std::string AuthService::login(const std::string& username, const std::string& password) {
    // 参数验证
    if (username.empty()) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Username cannot be empty");
    }
    if (password.empty()) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Password cannot be empty");
    }
    
    // 查找用户
    auto user = models::g_userRepository->getUserByUsername(username);
    if (!user) {
        common::g_logger.warning("Login attempt failed: User not found. Username: %s", username.c_str());
        throw common::AppException(common::ErrorCode::USER_NOT_FOUND, "Invalid username or password");
    }
    
    // 验证密码
    if (!verifyPassword(password, user->getPasswordHash())) {
        common::g_logger.warning("Login attempt failed: Invalid password. Username: %s", username.c_str());
        throw common::AppException(common::ErrorCode::INVALID_CREDENTIALS, "Invalid username or password");
    }
    
    // 生成token
    std::string token = generateToken(*user);
    
    common::g_logger.info("User logged in successfully. User ID: %lld, Username: %s", 
                     user->getId(), username.c_str());
    
    return token;
}

long long AuthService::registerUser(const std::string& username, const std::string& password) {
    // 参数验证
    if (username.empty()) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Username cannot be empty");
    }
    if (password.empty()) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Password cannot be empty");
    }
    
    // 验证用户名格式
    for (char c : username) {
        if (!std::isalnum(c) && c != '_' && c != '-') {
            throw common::AppException(common::ErrorCode::INVALID_PARAM, 
                                      "Username can only contain letters, numbers, underscores and hyphens");
        }
    }
    
    // 验证用户名长度
    if (username.length() < 3 || username.length() > 50) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, 
                                  "Username length must be between 3 and 50 characters");
    }
    
    // 验证密码长度
    if (password.length() < 6) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, 
                                  "Password length must be at least 6 characters");
    }
    
    // 检查用户名是否已存在
    if (models::g_userRepository->usernameExists(username)) {
        throw common::AppException(common::ErrorCode::USER_EXISTS, "Username already exists");
    }
    
    // 创建新用户
    models::User user;
    user.setUsername(username);
    user.setPasswordHash(models::User::hashPassword(password));
    
    // 保存用户
    bool success = models::g_userRepository->createUser(user);
    if (!success) {
        throw common::AppException(common::ErrorCode::INTERNAL_ERROR, "Failed to create user");
    }
    
    // 查询保存后的用户ID
    auto createdUser = models::g_userRepository->getUserByUsername(username);
    if (!createdUser) {
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Failed to retrieve created user");
    }
    
    common::g_logger.info("User registered successfully. User ID: %lld, Username: %s", 
                     createdUser->getId(), username.c_str());
    
    return createdUser->getId();
}

std::shared_ptr<models::User> AuthService::validateToken(const std::string& token) {
    if (token.empty()) {
        throw common::AppException(common::ErrorCode::UNAUTHORIZED, "Token is required");
    }
    
    try {
        // 验证并解析token
        std::unordered_map<std::string, std::string> payload = m_jwt->verifyAndParseToken(token);
        
        // 检查payload中是否包含user_id
        if (payload.find("user_id") == payload.end()) {
            throw common::AppException(common::ErrorCode::INVALID_TOKEN, "Invalid token payload");
        }
        
        // 获取用户ID
        long long userId = std::stoll(payload["user_id"]);
        
        // 查询用户
        auto user = models::g_userRepository->getUserById(userId);
        if (!user) {
            throw common::AppException(common::ErrorCode::USER_NOT_FOUND, "User not found");
        }   
        return user;
    } catch (const JWTException& e) {
        std::string errorMsg = e.what();
        if (errorMsg.find("Token has expired") != std::string::npos) {
            throw common::AppException(common::ErrorCode::TOKEN_EXPIRED, "Token has expired");
        } else {
            throw common::AppException(common::ErrorCode::INVALID_TOKEN, errorMsg);
        }
    } catch (const std::exception& e) {
        common::g_logger.error("Token validation error: %s", e.what());
        throw common::AppException(common::ErrorCode::INVALID_TOKEN, "Invalid token");
    }
}

bool AuthService::checkPermission(long long userId, const std::string& requiredPermission) {
    // 简化实现：目前所有注册用户都有相同权限
    // 实际应用中应实现基于角色的访问控制（RBAC）
    auto user = models::g_userRepository->getUserById(userId);
    if (!user) {
        throw common::AppException(common::ErrorCode::FORBIDDEN, "Permission denied");
    }
    
    // 可以扩展为基于用户角色的权限检查
    return true;
}

std::string AuthService::generateToken(const models::User& user) {
    std::unordered_map<std::string, std::string> payload;
    
    // 添加用户信息到payload
    payload["user_id"] = std::to_string(user.getId());
    
    // 从配置中获取token过期时间
    int expiryHours = common::g_config.getInt("jwt.expiry_hours", 24);
    
    // 生成token
    return m_jwt->generateToken(payload, expiryHours);
}

bool AuthService::verifyPassword(const std::string& password, const std::string& passwordHash) {
    // 简单的密码比较（实际项目中应该使用更安全的哈希比较）
    return password == passwordHash; // 临时实现
}

// 全局认证服务实例
std::shared_ptr<AuthService> g_authService;

bool initAuthService() {
    try {
        g_authService = std::make_shared<AuthService>();
        common::g_logger.info("%s", "Auth service initialized successfully");
        return true;
    } catch (const std::exception& e) {
        common::g_logger.error("Failed to initialize auth service: %s", e.what());
        return false;
    }
}

} // namespace auth
