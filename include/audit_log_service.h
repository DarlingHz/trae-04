// 审计日志服务头文件

#ifndef AUDIT_LOG_SERVICE_H
#define AUDIT_LOG_SERVICE_H

#include <string>
#include <vector>
#include <optional>
#include <memory>

#include "models.h"
#include "database.h"
#include "auth_service.h"

// 审计日志服务类
class AuditLogService {
public:
    // 构造函数
    AuditLogService(std::shared_ptr<Database> database, std::shared_ptr<AuthService> auth_service);
    
    // 析构函数
    ~AuditLogService();
    
    // 创建审计日志
    bool CreateAuditLog(int user_id, const std::string& action_type, const std::string& resource_type,
                         const std::optional<int>& resource_id = std::nullopt,
                         const std::optional<std::string>& detail = std::nullopt);
    
    // 获取用户的审计日志列表
    std::vector<AuditLog> GetUserAuditLogs(int user_id, int limit = 10);
    
    // 获取审计日志详情
    std::optional<AuditLog> GetAuditLogById(int user_id, int audit_log_id);
    
    // 记录用户注册日志
    bool LogUserRegister(int user_id, const std::string& email);
    
    // 记录用户登录日志
    bool LogUserLogin(int user_id, const std::string& email);
    
    // 记录项目创建日志
    bool LogProjectCreate(int user_id, int project_id, const std::string& project_name);
    
    // 记录项目更新日志
    bool LogProjectUpdate(int user_id, int project_id, const std::string& project_name);
    
    // 记录项目删除日志
    bool LogProjectDelete(int user_id, int project_id, const std::string& project_name);
    
    // 记录任务创建日志
    bool LogTaskCreate(int user_id, int task_id, const std::string& task_title);
    
    // 记录任务更新日志
    bool LogTaskUpdate(int user_id, int task_id, const std::string& task_title);
    
    // 记录任务删除日志
    bool LogTaskDelete(int user_id, int task_id, const std::string& task_title);
    
private:
    std::shared_ptr<Database> database_;  // 数据库访问对象
    std::shared_ptr<AuthService> auth_service_;  // 认证服务对象
};

// 审计日志服务异常类
class AuditLogServiceException : public std::exception {
public:
    explicit AuditLogServiceException(const std::string& message) : message_(message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
    
private:
    std::string message_;
};

#endif  // AUDIT_LOG_SERVICE_H