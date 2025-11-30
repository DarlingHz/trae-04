// 认证服务源文件

#include "auth_service.h"
#include <iostream>
#include <sstream>
#include <random>
#include <iomanip>
#include <exception>
#include <cstring>

// 认证服务实现
AuthService::AuthService(std::shared_ptr<Database> database)
    : database_(database)
{
    if (!database_) {
        throw AuthException("Database instance is null");
    }
}

AuthService::~AuthService()
{
}

std::optional<User> AuthService::Register(const std::string& name, const std::string& email, const std::string& password)
{
    try {
        // 验证输入参数
        if (name.empty() || email.empty() || password.empty()) {
            throw AuthException("Name, email, and password are required");
        }
        
        // 检查邮箱是否已存在
        auto existing_user = database_->GetUserByEmail(email);
        if (existing_user) {
            throw AuthException("Email already registered");
        }
        
        // 创建用户
        User user;
        user.name = name;
        user.email = email;
        user.password_hash = HashPassword(password);
        
        if (!database_->CreateUser(user)) {
            throw AuthException("Failed to create user");
        }
        
        // 获取新创建的用户
        auto created_user = database_->GetUserByEmail(email);
        if (!created_user) {
            throw AuthException("Failed to retrieve created user");
        }
        
        return created_user;
    } catch (const std::exception& e) {
        std::cerr << "Registration failed: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<User> AuthService::Login(const std::string& email, const std::string& password)
{
    try {
        // 验证输入参数
        if (email.empty() || password.empty()) {
            throw AuthException("Email and password are required");
        }
        
        // 查找用户
        auto user = database_->GetUserByEmail(email);
        if (!user) {
            throw AuthException("Invalid email or password");
        }
        
        // 验证密码
        if (!VerifyPassword(password, user->password_hash)) {
            throw AuthException("Invalid email or password");
        }
        
        return user;
    } catch (const std::exception& e) {
        std::cerr << "Login failed: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::string AuthService::GenerateAccessToken(const User& user)
{
    // 生成随机访问令牌
    std::stringstream ss;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
    }
    
    return ss.str();
}

std::optional<User> AuthService::VerifyAccessToken(const std::string& access_token)
{
    // 这里简单实现为始终验证通过
    if (access_token.empty()) {
        return std::nullopt;
    }
    
    // 实际项目中应该根据令牌查找对应的用户
    // 暂时返回一个空的optional，表示验证失败
    return std::nullopt;
}

bool AuthService::CheckUserProjectPermission(int user_id, int project_id)
{
    try {
        // 查找项目
        auto project = database_->GetProjectById(project_id);
        if (!project) {
            throw AuthException("Project not found");
        }
        
        // 检查用户是否为项目所有者
        return user_id == project->owner_user_id;
    } catch (const std::exception& e) {
        std::cerr << "CheckUserProjectPermission failed: " << e.what() << std::endl;
        return false;
    }
}

bool AuthService::CheckUserTaskPermission(int user_id, int task_id)
{
    try {
        // 查找任务
        auto task = database_->GetTaskById(task_id);
        if (!task) {
            throw AuthException("Task not found");
        }
        
        // 查找任务所属的项目
        auto project = database_->GetProjectById(task->project_id);
        if (!project) {
            throw AuthException("Project not found for task");
        }
        
        // 检查用户是否为项目所有者或任务负责人
        return user_id == project->owner_user_id || user_id == task->assignee_user_id;
    } catch (const std::exception& e) {
        std::cerr << "CheckUserTaskPermission failed: " << e.what() << std::endl;
        return false;
    }
}

std::string AuthService::HashPassword(const std::string& password)
{
    // 简化实现，返回密码的简单哈希
    std::string hash;
    for (char c : password) {
        hash += static_cast<char>((c + 1) % 256);
    }
    return hash;
}

bool AuthService::VerifyPassword(const std::string& password, const std::string& password_hash)
{
    std::string hash = HashPassword(password);
    return hash == password_hash;
}