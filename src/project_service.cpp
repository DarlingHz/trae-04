// 项目服务源文件

#include "project_service.h"
#include "utils.h"

#include <stdexcept>
#include <sstream>

// 项目服务实现
ProjectService::ProjectService(std::shared_ptr<Database> database, std::shared_ptr<AuthService> auth_service) 
    : database_(database), auth_service_(auth_service) {
    
    if (!database_) {
        throw ProjectServiceException("Database instance is null");
    }
    
    if (!auth_service_) {
        throw ProjectServiceException("AuthService instance is null");
    }
}

ProjectService::~ProjectService() {
}

std::optional<Project> ProjectService::CreateProject(int user_id, const std::string& name, const std::optional<std::string>& description) {
    // 验证输入参数
    if (name.empty()) {
        return std::nullopt;
    }
    
    // 创建项目
    Project project;
    project.id = 0;  // 数据库会自动生成ID
    project.owner_user_id = user_id;
    project.name = name;
    project.description = description;
    project.created_at = std::chrono::system_clock::now();
    
    if (!database_->CreateProject(project)) {
        return std::nullopt;
    }
    
    // 获取新创建的项目
    // 这里我们需要一种方法来获取刚创建的项目ID
    // 由于SQLite不支持LAST_INSERT_ID()的直接获取，我们可以通过查询最新创建的项目来获取
    std::string sql = "SELECT id, owner_user_id, name, description, created_at FROM projects WHERE owner_user_id = ? ORDER BY created_at DESC LIMIT 1";
    std::vector<std::string> params = {std::to_string(user_id)};
    
    auto projects = database_->ExecuteQuery<Project>(sql, params, Database::ParseProjectFromStatement);
    
    if (projects.empty()) {
        return std::nullopt;
    }
    
    return projects[0];
}

std::optional<Project> ProjectService::GetProjectById(int user_id, int project_id) {
    // 查找项目
    auto project = database_->GetProjectById(project_id);
    if (!project) {
        return std::nullopt;
    }
    
    // 检查用户是否有权限访问项目
    if (user_id != project->owner_user_id) {
        throw ProjectServiceException("Permission denied: You don't have access to this project");
    }
    
    return project;
}

std::vector<Project> ProjectService::GetProjectsByUserId(int user_id, int page, int page_size) {
    // 获取用户的项目列表
    return database_->GetProjectsByOwnerUserId(user_id, page, page_size);
}

int ProjectService::GetProjectsCountByUserId(int user_id) {
    // 获取用户的项目数量
    return database_->GetProjectsCountByOwnerUserId(user_id);
}

std::optional<Project> ProjectService::UpdateProject(int user_id, int project_id, const std::optional<std::string>& name, const std::optional<std::string>& description) {
    // 查找项目
    auto existing_project = database_->GetProjectById(project_id);
    if (!existing_project) {
        return std::nullopt;
    }
    
    // 检查用户是否有权限更新项目
    if (user_id != existing_project->owner_user_id) {
        return std::nullopt;
    }
    
    // 验证输入参数
    if (name && name->empty()) {
        return std::nullopt;
    }
    
    // 创建更新后的项目对象
    Project updated_project = *existing_project;
    if (name) {
        updated_project.name = *name;
    }
    if (description) {
        updated_project.description = *description;
    }
    
    // 更新项目
    if (!database_->UpdateProject(updated_project)) {
        return std::nullopt;
    }
    
    return updated_project;
}

bool ProjectService::DeleteProject(int user_id, int project_id) {
    // 查找项目
    auto project = database_->GetProjectById(project_id);
    if (!project) {
        return false;
    }
    
    // 检查用户是否有权限删除项目
    if (user_id != project->owner_user_id) {
        return false;
    }
    
    // 获取项目中的所有任务
    auto tasks = database_->GetTasksByProjectId(project_id, 1, 1000);  // 获取所有任务
    
    // 删除项目中的所有任务
    for (const auto& task : tasks) {
        // 删除任务的所有标签
        if (!database_->RemoveAllTagsFromTask(task.id)) {
            return false;
        }
        
        // 删除任务
        if (!database_->DeleteTask(task.id)) {
            return false;
        }
    }
    
    // 删除项目
    if (!database_->DeleteProject(project_id)) {
        return false;
    }
    
    return true;
}

std::optional<ProjectStats> ProjectService::GetProjectStats(int user_id, int project_id) {
    // 查找项目
    auto project = database_->GetProjectById(project_id);
    if (!project) {
        return std::nullopt;
    }
    
    if (user_id != project->owner_user_id) {
        return std::nullopt;
    }
    
    // 获取项目统计信息
    return database_->GetProjectStats(project_id);
}