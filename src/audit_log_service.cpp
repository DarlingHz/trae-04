// 审计日志服务源文件

#include "audit_log_service.h"

#include <stdexcept>
#include <sstream>

// 审计日志服务实现
AuditLogService::AuditLogService(std::shared_ptr<Database> database, std::shared_ptr<AuthService> auth_service) : database_(database), auth_service_(auth_service) {
    if (!database_) {
        throw AuditLogServiceException("Database instance is null");
    }
}

AuditLogService::~AuditLogService() {
}

bool AuditLogService::CreateAuditLog(int user_id, const std::string& action_type, const std::string& resource_type, const std::optional<int>& resource_id, const std::optional<std::string>& detail) {
    // 验证输入参数
    if (action_type.empty() || resource_type.empty()) {
        throw AuditLogServiceException("Action type and resource type are required");
    }
    
    // 创建审计日志
    AuditLog audit_log;
    audit_log.user_id = user_id;
    audit_log.action_type = action_type;
    audit_log.resource_type = resource_type;
    audit_log.resource_id = resource_id;
    audit_log.created_at = std::chrono::system_clock::now();
    audit_log.detail = detail;
    
    return database_->CreateAuditLog(audit_log);
}

std::optional<AuditLog> AuditLogService::GetAuditLogById(int user_id, int audit_log_id) {
    // 查找审计日志
    auto audit_log = database_->GetAuditLogById(audit_log_id);
    if (!audit_log) {
        return std::nullopt;
    }
    
    // 检查用户是否有权限访问该审计日志
    if (user_id != audit_log->user_id) {
        throw AuditLogServiceException("Permission denied: You don't have access to this audit log");
    }
    
    return audit_log;
}

std::vector<AuditLog> AuditLogService::GetUserAuditLogs(int user_id, int limit) {
    // 验证输入参数
    if (limit <= 0) {
        throw AuditLogServiceException("Limit must be a positive integer");
    }
    
    // 获取用户的审计日志列表
    return database_->GetAuditLogsByUserId(user_id, limit);
}

bool AuditLogService::LogUserRegister(int user_id, const std::string& email) {
    // 记录用户注册日志
    std::string detail = "User registered with email: " + email;
    return CreateAuditLog(user_id, "user_register", "user", user_id, detail);
}

bool AuditLogService::LogUserLogin(int user_id, const std::string& email) {
    // 记录用户登录日志
    std::string detail = "User logged in with email: " + email;
    return CreateAuditLog(user_id, "user_login", "user", user_id, detail);
}

bool AuditLogService::LogProjectCreate(int user_id, int project_id, const std::string& project_name) {
    // 记录项目创建日志
    std::string detail = "Created project: " + project_name;
    return CreateAuditLog(user_id, "create_project", "project", project_id, detail);
}

bool AuditLogService::LogProjectUpdate(int user_id, int project_id, const std::string& project_name) {
    // 记录项目更新日志
    std::string detail = "Updated project: " + project_name;
    return CreateAuditLog(user_id, "update_project", "project", project_id, detail);
}

bool AuditLogService::LogProjectDelete(int user_id, int project_id, const std::string& project_name) {
    // 记录项目删除日志
    std::string detail = "Deleted project: " + project_name;
    return CreateAuditLog(user_id, "delete_project", "project", project_id, detail);
}

bool AuditLogService::LogTaskCreate(int user_id, int task_id, const std::string& task_title) {
    // 记录任务创建日志
    std::string detail = "Created task: " + task_title;
    return CreateAuditLog(user_id, "create_task", "task", task_id, detail);
}

bool AuditLogService::LogTaskUpdate(int user_id, int task_id, const std::string& task_title) {
    // 记录任务更新日志
    std::string detail = "Updated task: " + task_title;
    return CreateAuditLog(user_id, "update_task", "task", task_id, detail);
}

bool AuditLogService::LogTaskDelete(int user_id, int task_id, const std::string& task_title) {
    // 记录任务删除日志
    std::string detail = "Deleted task: " + task_title;
    return CreateAuditLog(user_id, "delete_task", "task", task_id, detail);
}