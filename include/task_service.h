// 任务服务头文件

#ifndef TASK_SERVICE_H
#define TASK_SERVICE_H

#include <string>
#include <vector>
#include <optional>
#include <memory>

#include "models.h"
#include "database.h"
#include "auth_service.h"

// 任务服务类
class TaskService {
public:
    // 构造函数
    TaskService(std::shared_ptr<Database> database, std::shared_ptr<AuthService> auth_service);
    
    // 析构函数
    ~TaskService();
    
    // 创建任务
    std::optional<Task> CreateTask(int user_id, int project_id, const std::string& title,
                                      const std::optional<std::string>& description = std::nullopt,
                                      const std::optional<int>& assignee_user_id = std::nullopt,
                                      const std::string& status = "todo",
                                      const std::string& priority = "medium",
                                      const std::optional<std::chrono::system_clock::time_point>& due_date = std::nullopt,
                                      const std::vector<std::string>& tags = {});
    
    // 获取任务详情
    std::optional<Task> GetTaskById(int user_id, int task_id);
    
    // 获取项目中的任务列表
    std::vector<Task> GetTasksByProjectId(int user_id, int project_id, int page = 1, int page_size = 10);
    
    // 根据查询参数获取任务列表
    std::vector<Task> GetTasksByQueryParams(int user_id, const TaskQueryParams& params);
    
    // 获取项目中的任务数量
    int GetTasksCountByProjectId(int user_id, int project_id);
    
    // 根据查询参数获取任务数量
    int GetTasksCountByQueryParams(int user_id, const TaskQueryParams& params);
    
    // 更新任务
    std::optional<Task> UpdateTask(int user_id, int task_id, const std::optional<std::string>& title = std::nullopt,
                                      const std::optional<std::string>& description = std::nullopt,
                                      const std::optional<int>& assignee_user_id = std::nullopt,
                                      const std::optional<std::string>& status = std::nullopt,
                                      const std::optional<std::string>& priority = std::nullopt,
                                      const std::optional<std::chrono::system_clock::time_point>& due_date = std::nullopt,
                                      const std::optional<std::vector<std::string>>& tags = std::nullopt);
    
    // 删除任务
    bool DeleteTask(int user_id, int task_id);
    
    // 获取任务的标签列表
    std::vector<Tag> GetTaskTags(int user_id, int task_id);
    
    // 为任务添加标签
    bool AddTagToTask(int user_id, int task_id, const std::string& tag_name);
    
    // 从任务中移除标签
    bool RemoveTagFromTask(int user_id, int task_id, const std::string& tag_name);
    
    // 验证任务状态流转是否合法
    bool IsValidTaskStatusTransition(const std::string& from_status, const std::string& to_status);
    
    // 搜索任务
    std::vector<Task> SearchTasks(int user_id, const TaskQueryParams& params);
    
    // 获取任务搜索结果数量
    int GetSearchTasksCount(int user_id, const TaskQueryParams& params);
    
private:
    std::shared_ptr<Database> database_;  // 数据库访问对象
    std::shared_ptr<AuthService> auth_service_;  // 认证服务对象
    
    // 检查用户是否有权限访问任务
    bool IsUserAuthorizedForTask(int user_id, const Task& task);
    
    // 处理任务标签
    bool ProcessTaskTags(int task_id, const std::vector<std::string>& tags);
};

// 任务服务异常类
class TaskServiceException : public std::exception {
public:
    explicit TaskServiceException(const std::string& message) : message_(message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
    
private:
    std::string message_;
};

#endif  // TASK_SERVICE_H