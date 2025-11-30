// 项目服务头文件

#ifndef PROJECT_SERVICE_H
#define PROJECT_SERVICE_H

#include <string>
#include <vector>
#include <optional>
#include <memory>

#include "models.h"
#include "database.h"
#include "auth_service.h"

// 项目服务类
class ProjectService {
public:
    // 构造函数
    ProjectService(std::shared_ptr<Database> database, std::shared_ptr<AuthService> auth_service);
    
    // 析构函数
    ~ProjectService();
    
    // 创建项目
    std::optional<Project> CreateProject(int user_id, const std::string& name,
                                             const std::optional<std::string>& description = std::nullopt);
    
    // 获取项目详情
    std::optional<Project> GetProjectById(int user_id, int project_id);
    
    // 获取用户的项目列表
    std::vector<Project> GetProjectsByUserId(int user_id, int page = 1, int page_size = 10);
    
    // 获取用户的项目数量
    int GetProjectsCountByUserId(int user_id);
    
    // 更新项目
    std::optional<Project> UpdateProject(int user_id, int project_id, const std::optional<std::string>& name = std::nullopt,
                                             const std::optional<std::string>& description = std::nullopt);
    
    // 删除项目
    bool DeleteProject(int user_id, int project_id);
    
    // 获取项目统计信息
    std::optional<ProjectStats> GetProjectStats(int user_id, int project_id);
    
private:
    std::shared_ptr<Database> database_;  // 数据库访问对象
    std::shared_ptr<AuthService> auth_service_;  // 认证服务对象
};

// 项目服务异常类
class ProjectServiceException : public std::exception {
public:
    explicit ProjectServiceException(const std::string& message) : message_(message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
    
private:
    std::string message_;
};

#endif  // PROJECT_SERVICE_H