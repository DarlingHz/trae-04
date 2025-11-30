// 任务服务源文件

#include "task_service.h"
#include "utils.h"

#include <stdexcept>
#include <sstream>
#include <algorithm>

// 任务服务实现
TaskService::TaskService(std::shared_ptr<Database> database, std::shared_ptr<AuthService> auth_service) 
    : database_(database), auth_service_(auth_service) {
    
    if (!database_) {
        throw TaskServiceException("Database instance is null");
    }
    
    if (!auth_service_) {
        throw TaskServiceException("AuthService instance is null");
    }
}

TaskService::~TaskService() {
}

std::optional<Task> TaskService::CreateTask(int user_id, int project_id, const std::string& title, const std::optional<std::string>& description, const std::optional<int>& assignee_user_id, const std::string& status, const std::string& priority, const std::optional<std::chrono::system_clock::time_point>& due_date, const std::vector<std::string>& tags) {
    // 验证输入参数
    if (title.empty()) {
        return std::nullopt;
    }
    
    // 检查项目是否存在
    auto project = database_->GetProjectById(project_id);
    if (!project) {
        return std::nullopt;
    }
    
    // 检查用户是否有权限创建任务（必须是项目所有者）
    if (user_id != project->owner_user_id) {
        return std::nullopt;
    }
    
    // 检查负责人是否存在（如果指定了负责人）
    if (assignee_user_id) {
        auto assignee = database_->GetUserById(assignee_user_id.value());
        if (!assignee) {
            return std::nullopt;
        }
    }
    
    // 创建任务
    Task task;
    task.id = 0;  // 数据库会自动生成ID
    task.project_id = project_id;
    task.assignee_user_id = assignee_user_id;
    task.title = title;
    task.description = description;
    task.status = status;
    task.priority = priority;
    task.due_date = due_date;
    task.created_at = std::chrono::system_clock::now();
    task.updated_at = task.created_at;
    
    if (!database_->CreateTask(task)) {
        return std::nullopt;
    }
    
    // 获取新创建的任务
    // 这里我们需要一种方法来获取刚创建的任务ID
    // 由于SQLite不支持LAST_INSERT_ID()的直接获取，我们可以通过查询最新创建的任务来获取
    std::string sql = "SELECT id, project_id, assignee_user_id, title, description, status, priority, due_date, created_at, updated_at FROM tasks WHERE project_id = ? ORDER BY created_at DESC LIMIT 1";
    std::vector<std::string> params = {std::to_string(project_id)};
    
    auto tasks = database_->ExecuteQuery<Task>(sql, params, Database::ParseTaskFromStatement);
    
    if (tasks.empty()) {
        return std::nullopt;
    }
    
    Task created_task = tasks[0];
    
    // 处理标签（如果有）
    if (!tags.empty()) {
        for (const auto& tag_name : tags) {
            // 查找或创建标签
            auto tag = database_->GetTagByName(tag_name);
            if (!tag) {
                Tag new_tag;
                new_tag.name = tag_name;
                
                if (!database_->CreateTag(new_tag)) {
                    return std::nullopt;
                }
                
                tag = database_->GetTagByName(tag_name);
                if (!tag) {
                    return std::nullopt;
                }
            }
            
            // 将标签添加到任务
            if (!database_->AddTagToTask(created_task.id, tag->id)) {
                return std::nullopt;
            }
        }
    }
    
    return created_task;
}

// 检查用户是否有权限访问任务


std::optional<Task> TaskService::GetTaskById(int user_id, int task_id) {
    // 查找任务
    auto task = database_->GetTaskById(task_id);
    if (!task) {
        return std::nullopt;
    }
    
    // 检查用户是否有权限访问任务
    if (!IsUserAuthorizedForTask(user_id, *task)) {
        return std::nullopt;
    }
    
    return task;
}

std::vector<Task> TaskService::GetTasksByProjectId(int user_id, int project_id, int page, int page_size) {
    // 检查项目是否存在
    auto project = database_->GetProjectById(project_id);
    if (!project) {
        throw TaskServiceException("Project not found");
    }
    
    // 检查用户是否有权限访问项目
    if (user_id != project->owner_user_id) {
        throw TaskServiceException("Permission denied: You don't have access to this project");
    }
    
    // 获取项目中的任务
    auto tasks = database_->GetTasksByProjectId(project_id, page, page_size);
    
    return tasks;
}

std::vector<Task> TaskService::SearchTasks(int user_id, const TaskQueryParams& params) {
    // 检查用户是否有权限访问搜索结果中的任务
    // 由于搜索可能跨多个项目，我们需要确保用户只能看到自己有权限的项目中的任务
    // 这里我们可以通过在查询参数中添加用户有权限的项目ID列表来实现
    
    // 获取用户有权限的项目ID列表
    auto projects = database_->GetProjectsByOwnerUserId(user_id, 1, 1000);  // 获取所有项目
    std::vector<int> project_ids;
    for (const auto& project : projects) {
        project_ids.push_back(project.id);
    }
    
    if (project_ids.empty()) {
        // 用户没有任何项目，返回空列表
        return {};
    }
    
    // 构建SQL查询
    std::string sql = "SELECT DISTINCT t.id, t.project_id, t.assignee_user_id, t.title, t.description, t.status, t.priority, t.due_date, t.created_at, t.updated_at FROM tasks t LEFT JOIN task_tags tt ON t.id = tt.task_id LEFT JOIN tags tag ON tt.tag_id = tag.id WHERE t.project_id IN (";
    
    // 添加项目ID列表
    for (int i = 0; i < project_ids.size(); ++i) {
        if (i > 0) {
            sql += ",";
        }
        sql += std::to_string(project_ids[i]);
    }
    
    sql += ")";
    
    std::vector<std::string> sql_params;
    
    // 添加状态过滤
    if (params.status) {
        sql += " AND t.status = ?";
        sql_params.push_back(params.status.value());
    }
    
    // 添加优先级过滤
    if (params.priority) {
        sql += " AND t.priority = ?";
        sql_params.push_back(params.priority.value());
    }
    
    // 添加截止日期之前过滤
    if (params.due_before) {
        sql += " AND t.due_date <= ?";
        std::stringstream due_before_ss;
        due_before_ss << std::chrono::system_clock::to_time_t(params.due_before.value());
        sql_params.push_back(due_before_ss.str());
    }
    
    // 添加截止日期之后过滤
    if (params.due_after) {
        sql += " AND t.due_date >= ?";
        std::stringstream due_after_ss;
        due_after_ss << std::chrono::system_clock::to_time_t(params.due_after.value());
        sql_params.push_back(due_after_ss.str());
    }
    
    // 添加关键词过滤
    if (params.keyword) {
        sql += " AND (t.title LIKE ? OR t.description LIKE ?)";
        sql_params.push_back("%" + params.keyword.value() + "%");
        sql_params.push_back("%" + params.keyword.value() + "%");
    }
    
    // 添加标签过滤
    if (params.tag) {
        // 当添加标签过滤条件时，使用INNER JOIN语法，这样只会返回有标签的任务
        sql = "SELECT DISTINCT t.id, t.project_id, t.assignee_user_id, t.title, t.description, t.status, t.priority, t.due_date, t.created_at, t.updated_at FROM tasks t INNER JOIN task_tags tt ON t.id = tt.task_id INNER JOIN tags tag ON tt.tag_id = tag.id WHERE t.project_id IN (";
        
        // 重新添加项目ID列表
        for (int i = 0; i < project_ids.size(); ++i) {
            if (i > 0) {
                sql += ",";
            }
            sql += std::to_string(project_ids[i]);
        }
        
        sql += ")";
        
        // 清空sql_params，因为我们要重新添加所有参数
        sql_params.clear();
        
        // 重新添加之前的过滤条件和参数
        if (params.status) {
            sql += " AND t.status = ?";
            sql_params.push_back(params.status.value());
        }
        
        if (params.priority) {
            sql += " AND t.priority = ?";
            sql_params.push_back(params.priority.value());
        }
        
        if (params.due_before) {
            sql += " AND t.due_date <= ?";
            std::stringstream due_before_ss;
            due_before_ss << std::chrono::system_clock::to_time_t(params.due_before.value());
            sql_params.push_back(due_before_ss.str());
        }
        
        if (params.due_after) {
            sql += " AND t.due_date >= ?";
            std::stringstream due_after_ss;
            due_after_ss << std::chrono::system_clock::to_time_t(params.due_after.value());
            sql_params.push_back(due_after_ss.str());
        }
        
        if (params.keyword) {
            sql += " AND (t.title LIKE ? OR t.description LIKE ?)";
            sql_params.push_back("%" + params.keyword.value() + "%");
            sql_params.push_back("%" + params.keyword.value() + "%");
        }
        
        // 添加标签过滤条件和参数
        sql += " AND tag.name = ?";
        sql_params.push_back(params.tag.value());
    }
    
    // 添加排序
    sql += " ORDER BY t.created_at DESC";
    
    // 添加分页
    if (params.page_size > 0) {
        sql += " LIMIT ? OFFSET ?";
        sql_params.push_back(std::to_string(params.page_size));
        sql_params.push_back(std::to_string((params.page - 1) * params.page_size));
    }
    
    // 执行查询
    auto tasks = database_->ExecuteQuery<Task>(sql, sql_params, Database::ParseTaskFromStatement);
    
    return tasks;
}

int TaskService::GetSearchTasksCount(int user_id, const TaskQueryParams& params) {
    // 获取用户有权限的项目ID列表
    auto projects = database_->GetProjectsByOwnerUserId(user_id, 1, 1000);  // 获取所有项目
    std::vector<int> project_ids;
    for (const auto& project : projects) {
        project_ids.push_back(project.id);
    }
    
    if (project_ids.empty()) {
        // 用户没有任何项目，返回0
        return 0;
    }
    
    // 计算所有项目中匹配任务的总数
    int total_count = 0;
    for (int project_id : project_ids) {
        TaskQueryParams project_params = params;
        project_params.project_id = project_id;
        
        total_count += database_->GetTasksCountByQueryParams(project_params);
    }
    
    return total_count;
}

std::optional<Task> TaskService::UpdateTask(int user_id, int task_id, const std::optional<std::string>& title, const std::optional<std::string>& description, const std::optional<int>& assignee_user_id, const std::optional<std::string>& status, const std::optional<std::string>& priority, const std::optional<std::chrono::system_clock::time_point>& due_date, const std::optional<std::vector<std::string>>& tags) {
    // 查找任务
    auto existing_task = database_->GetTaskById(task_id);
    if (!existing_task) {
        return std::nullopt;
    }
    
    // 检查用户是否有权限更新任务
    if (!IsUserAuthorizedForTask(user_id, *existing_task)) {
        return std::nullopt;
    }
    
    // 检查负责人是否存在（如果指定了负责人）
    if (assignee_user_id) {
        auto assignee = database_->GetUserById(assignee_user_id.value());
        if (!assignee) {
            return std::nullopt;
        }
    }
    
    // 创建更新后的任务对象
    Task updated_task = *existing_task;
    
    if (title) {
        updated_task.title = title.value();
    }
    
    if (description) {
        updated_task.description = description.value();
    }
    
    if (status) {
        updated_task.status = status.value();
    }
    
    if (priority) {
        updated_task.priority = priority.value();
    }
    
    if (assignee_user_id) {
        updated_task.assignee_user_id = assignee_user_id.value();
    }
    
    if (due_date) {
        updated_task.due_date = due_date.value();
    }
    
    updated_task.updated_at = std::chrono::system_clock::now();
    
    // 更新任务
    if (!database_->UpdateTask(updated_task)) {
        return std::nullopt;
    }
    
    // 处理标签（如果提供了标签）
    if (tags) {
        // 移除任务的所有现有标签
        if (!database_->RemoveAllTagsFromTask(task_id)) {
            return std::nullopt;
        }
        
        // 添加新标签
        for (const auto& tag_name : tags.value()) {
            // 查找或创建标签
            auto tag = database_->GetTagByName(tag_name);
            if (!tag) {
                Tag new_tag;
                new_tag.name = tag_name;
                
                if (!database_->CreateTag(new_tag)) {
                    return std::nullopt;
                }
                
                tag = database_->GetTagByName(tag_name);
                if (!tag) {
                    return std::nullopt;
                }
            }
            
            // 将标签添加到任务
            if (!database_->AddTagToTask(task_id, tag->id)) {
                return std::nullopt;
            }
        }
    }
    
    return updated_task;
}

bool TaskService::DeleteTask(int user_id, int task_id) {
    // 查找任务
    auto task = database_->GetTaskById(task_id);
    if (!task) {
        return false;
    }
    
    // 检查用户是否有权限删除任务
    if (!IsUserAuthorizedForTask(user_id, *task)) {
        throw TaskServiceException("Permission denied: You don't have access to this task");
    }
    
    // 删除任务的所有标签
    if (!database_->RemoveAllTagsFromTask(task_id)) {
        throw TaskServiceException("Failed to remove tags from task");
    }
    
    // 删除任务
    if (!database_->DeleteTask(task_id)) {
        return false;
    }
    

    
    return true;
}

std::vector<Tag> TaskService::GetTaskTags(int user_id, int task_id) {
    // 查找任务
    auto task = database_->GetTaskById(task_id);
    if (!task) {
        return {};
    }
    
    // 检查用户是否有权限访问任务
    if (!IsUserAuthorizedForTask(user_id, *task)) {
        return {};
    }
    
    return database_->GetTagsByTaskId(task_id);
}

bool TaskService::IsValidTaskStatusTransition(const std::string& from_status, const std::string& to_status) {
    // 允许的状态流转：
    // todo -> doing
    // doing -> done
    // todo -> done (直接完成)
    // doing -> todo (回退到待办)
    // 不允许 done -> todo 或 done -> doing
    
    if (from_status == "done") {
        // 已完成的任务不能再修改状态
        return false;
    }
    
    return true;
}

bool TaskService::IsUserAuthorizedForTask(int user_id, const Task& task) {
    // 检查用户是否有权限访问任务
    // 用户必须是任务所属项目的所有者
    
    auto project = database_->GetProjectById(task.project_id);
    if (!project) {
        // 项目不存在，用户无权访问
        return false;
    }
    
    return user_id == project->owner_user_id;
}

bool TaskService::ProcessTaskTags(int task_id, const std::vector<std::string>& tags) {
    // 移除任务的所有现有标签
    if (!database_->RemoveAllTagsFromTask(task_id)) {
        return false;
    }
    
    // 添加新标签
    for (const auto& tag_name : tags) {
        // 查找或创建标签
        auto tag = database_->GetTagByName(tag_name);
        if (!tag) {
            Tag new_tag;
            new_tag.name = tag_name;
            
            if (!database_->CreateTag(new_tag)) {
                return false;
            }
            
            tag = database_->GetTagByName(tag_name);
            if (!tag) {
                return false;
            }
        }
        
        // 将标签添加到任务
        if (!database_->AddTagToTask(task_id, tag->id)) {
            return false;
        }
    }
    
    return true;
}