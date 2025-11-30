// 认证服务头文件

#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <string>
#include <optional>
#include <memory>

#include "models.h"
#include "database.h"

// 认证服务类
class AuthService {
public:
    // 构造函数
    explicit AuthService(std::shared_ptr<Database> database);
    
    // 析构函数
    ~AuthService();
    
    // 用户注册
    std::optional<User> Register(const std::string& name, const std::string& email, const std::string& password);
    
    // 用户登录
    std::optional<User> Login(const std::string& email, const std::string& password);
    
    // 生成访问令牌
    std::string GenerateAccessToken(const User& user);
    
    // 验证访问令牌
    std::optional<User> VerifyAccessToken(const std::string& access_token);
    
    // 检查用户是否有权限访问项目
    bool CheckUserProjectPermission(int user_id, int project_id);
    
    // 检查用户是否有权限访问任务
    bool CheckUserTaskPermission(int user_id, int task_id);
    
    // 哈希密码
    std::string HashPassword(const std::string& password);
    
    // 验证密码
    bool VerifyPassword(const std::string& password, const std::string& password_hash);
    
private:
    std::shared_ptr<Database> database_;  // 数据库访问对象
};

// 认证异常类
class AuthException : public std::exception {
public:
    explicit AuthException(const std::string& message) : message_(message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
    
private:
    std::string message_;
};

#endif  // AUTH_SERVICE_H