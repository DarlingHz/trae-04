// 数据模型头文件

#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>
#include <optional>
#include <chrono>

// 用户数据模型
struct User {
    int id;  // 主键
    std::string name;  // 用户名
    std::string email;  // 邮箱（唯一）
    std::string password_hash;  // 密码哈希值
    std::chrono::system_clock::time_point created_at;  // 创建时间
};

// 项目数据模型
struct Project {
    int id;  // 主键
    int owner_user_id;  // 所有者用户ID（关联User表）
    std::string name;  // 项目名称
    std::optional<std::string> description;  // 项目描述（可选）
    std::chrono::system_clock::time_point created_at;  // 创建时间
};

// 任务数据模型
struct Task {
    int id;  // 主键
    int project_id;  // 项目ID（关联Project表）
    std::optional<int> assignee_user_id;  // 负责人用户ID（关联User表，可选）
    std::string title;  // 任务标题
    std::optional<std::string> description;  // 任务描述（可选）
    std::string status;  // 任务状态（todo/doing/done）
    std::string priority;  // 任务优先级（low/medium/high）
    std::optional<std::chrono::system_clock::time_point> due_date;  // 截止日期（可选）
    std::chrono::system_clock::time_point created_at;  // 创建时间
    std::chrono::system_clock::time_point updated_at;  // 更新时间
};

// 标签数据模型
struct Tag {
    int id;  // 主键
    std::string name;  // 标签名称（唯一）
};

// 任务标签关联数据模型
struct TaskTag {
    int task_id;  // 任务ID（关联Task表）
    int tag_id;  // 标签ID（关联Tag表）
};

// 审计日志数据模型
struct AuditLog {
    int id;  // 主键
    int user_id;  // 用户ID（关联User表）
    std::string action_type;  // 操作类型（如create_task/update_task/login等）
    std::string resource_type;  // 资源类型（如task/project/user等）
    std::optional<int> resource_id;  // 资源ID（可选）
    std::chrono::system_clock::time_point created_at;  // 创建时间
    std::optional<std::string> detail;  // 详细信息（文本或JSON字符串，可选）
};

// 任务统计数据模型
struct TaskStats {
    int todo;  // 待办任务数量
    int doing;  // 进行中任务数量
    int done;  // 已完成任务数量
};

// 项目统计数据模型
struct ProjectStats {
    int total_tasks;  // 总任务数量
    int todo_tasks;  // 待办任务数量
    int doing_tasks;  // 进行中任务数量
    int done_tasks;  // 已完成任务数量
};

// 用户统计数据模型
struct UserStats {
    TaskStats task_stats;  // 任务状态统计
    int overdue_tasks;  // 逾期任务数量
    int recent_tasks;  // 近7天内新建任务数量
    int total_projects;  // 总项目数量
};

// 任务查询参数数据模型
struct TaskQueryParams {
    std::optional<int> project_id;  // 项目ID（可选）
    std::optional<int> assignee_user_id;  // 负责人用户ID（可选）
    std::optional<std::string> status;  // 任务状态（可选）
    std::optional<std::string> priority;  // 任务优先级（可选）
    std::optional<std::chrono::system_clock::time_point> due_before;  // 截止日期之前（可选）
    std::optional<std::chrono::system_clock::time_point> due_after;  // 截止日期之后（可选）
    std::optional<std::string> keyword;  // 关键字（匹配标题或描述，可选）
    std::optional<std::string> tag;  // 标签名称（可选）
    int page;  // 页码（默认1）
    int page_size;  // 每页数量（默认10，最大100）
};

#endif  // MODELS_H