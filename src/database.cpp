// 数据库访问层源文件

#include "database.h"
#include "utils.h"

#include <stdexcept>
#include <sstream>
#include <iomanip>

// 数据库连接池实现
DatabaseConnectionPool::DatabaseConnectionPool(const std::string& db_path, int pool_size)
    : db_path_(db_path), pool_size_(pool_size) {
    
    // 初始化连接池
    for (int i = 0; i < pool_size_; ++i) {
        sqlite3* conn = nullptr;
        int rc = sqlite3_open(db_path_.c_str(), &conn);
        
        if (rc != SQLITE_OK) {
            std::string error_message = sqlite3_errmsg(conn);
            sqlite3_close(conn);
            throw DatabaseException("Failed to open database connection: " + error_message);
        }
        
        // 设置数据库连接的超时时间
        sqlite3_busy_timeout(conn, 5000);  // 5秒
        
        connections_.push(std::shared_ptr<sqlite3>(conn, sqlite3_close));
    }
}

DatabaseConnectionPool::~DatabaseConnectionPool() {
    // 关闭所有连接
    while (!connections_.empty()) {
        connections_.pop();
    }
}

std::shared_ptr<sqlite3> DatabaseConnectionPool::GetConnection() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (connections_.empty()) {
        throw DatabaseException("No available database connections in pool");
    }
    
    std::shared_ptr<sqlite3> conn = connections_.front();
    connections_.pop();
    
    return conn;
}

void DatabaseConnectionPool::ReleaseConnection(std::shared_ptr<sqlite3> conn) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (conn) {
        connections_.push(conn);
    }
}

// 数据库访问实现
Database::Database(const std::string& db_path, int pool_size) {
    connection_pool_ = std::make_shared<DatabaseConnectionPool>(db_path, pool_size);
}

Database::~Database() {
}

// 用户相关操作
std::optional<User> Database::GetUserById(int id) {
    std::string sql = "SELECT id, name, email, password_hash, created_at FROM users WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};
    
    auto users = ExecuteQuery<User>(sql, params, ParseUserFromStatement);
    
    if (users.empty()) {
        return std::nullopt;
    }
    
    return users[0];
}

std::optional<User> Database::GetUserByEmail(const std::string& email) {
    std::string sql = "SELECT id, name, email, password_hash, created_at FROM users WHERE email = ?";
    std::vector<std::string> params = {email};
    
    auto users = ExecuteQuery<User>(sql, params, ParseUserFromStatement);
    
    if (users.empty()) {
        return std::nullopt;
    }
    
    return users[0];
}

bool Database::CreateUser(const User& user) {
    std::string sql = "INSERT INTO users (name, email, password_hash, created_at) VALUES (?, ?, ?, ?)";
    
    std::stringstream created_at_ss;
    created_at_ss << std::chrono::system_clock::to_time_t(user.created_at);
    
    std::vector<std::string> params = {
        user.name,
        user.email,
        user.password_hash,
        created_at_ss.str()
    };
    
    return ExecuteNonQuery(sql, params);
}

bool Database::UpdateUser(const User& user) {
    std::string sql = "UPDATE users SET name = ?, email = ?, password_hash = ? WHERE id = ?";
    
    std::vector<std::string> params = {
        user.name,
        user.email,
        user.password_hash,
        std::to_string(user.id)
    };
    
    return ExecuteNonQuery(sql, params);
}

bool Database::DeleteUser(int id) {
    std::string sql = "DELETE FROM users WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};
    
    return ExecuteNonQuery(sql, params);
}

// 项目相关操作
std::optional<Project> Database::GetProjectById(int id) {
    std::string sql = "SELECT id, owner_user_id, name, description, created_at FROM projects WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};
    
    auto projects = ExecuteQuery<Project>(sql, params, ParseProjectFromStatement);
    
    if (projects.empty()) {
        return std::nullopt;
    }
    
    return projects[0];
}

std::vector<Project> Database::GetProjectsByOwnerUserId(int owner_user_id, int page, int page_size) {
    int offset = (page - 1) * page_size;
    
    std::string sql = "SELECT id, owner_user_id, name, description, created_at FROM projects WHERE owner_user_id = ? LIMIT ? OFFSET ?";
    std::vector<std::string> params = {
        std::to_string(owner_user_id),
        std::to_string(page_size),
        std::to_string(offset)
    };
    
    return ExecuteQuery<Project>(sql, params, ParseProjectFromStatement);
}

int Database::GetProjectsCountByOwnerUserId(int owner_user_id) {
    std::string sql = "SELECT COUNT(*) FROM projects WHERE owner_user_id = ?";
    std::vector<std::string> params = {std::to_string(owner_user_id)};
    
    auto results = ExecuteQuery<int>(sql, params, [](sqlite3_stmt* stmt) -> int {
        return sqlite3_column_int(stmt, 0);
    });
    
    if (results.empty()) {
        return 0;
    }
    
    return results[0];
}

bool Database::CreateProject(const Project& project) {
    std::string sql = "INSERT INTO projects (owner_user_id, name, description, created_at) VALUES (?, ?, ?, ?)";
    
    std::stringstream created_at_ss;
    created_at_ss << std::chrono::system_clock::to_time_t(project.created_at);
    
    std::vector<std::string> params = {
        std::to_string(project.owner_user_id),
        project.name,
        project.description.value_or(""),
        created_at_ss.str()
    };
    
    return ExecuteNonQuery(sql, params);
}

bool Database::UpdateProject(const Project& project) {
    std::string sql = "UPDATE projects SET name = ?, description = ? WHERE id = ?";
    
    std::vector<std::string> params = {
        project.name,
        project.description.value_or(""),
        std::to_string(project.id)
    };
    
    return ExecuteNonQuery(sql, params);
}

bool Database::DeleteProject(int id) {
    std::string sql = "DELETE FROM projects WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};
    
    return ExecuteNonQuery(sql, params);
}

// 任务相关操作
std::optional<Task> Database::GetTaskById(int id) {
    std::string sql = "SELECT id, project_id, assignee_user_id, title, description, status, priority, due_date, created_at, updated_at FROM tasks WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};
    
    auto tasks = ExecuteQuery<Task>(sql, params, ParseTaskFromStatement);
    
    if (tasks.empty()) {
        return std::nullopt;
    }
    
    return tasks[0];
}

std::vector<Task> Database::GetTasksByProjectId(int project_id, int page, int page_size) {
    int offset = (page - 1) * page_size;
    
    std::string sql = "SELECT id, project_id, assignee_user_id, title, description, status, priority, due_date, created_at, updated_at FROM tasks WHERE project_id = ? LIMIT ? OFFSET ?";
    std::vector<std::string> params = {
        std::to_string(project_id),
        std::to_string(page_size),
        std::to_string(offset)
    };
    
    return ExecuteQuery<Task>(sql, params, ParseTaskFromStatement);
}

std::vector<Task> Database::GetTasksByQueryParams(const TaskQueryParams& params) {
    std::string sql = "SELECT DISTINCT t.id, t.project_id, t.assignee_user_id, t.title, t.description, t.status, t.priority, t.due_date, t.created_at, t.updated_at FROM tasks t LEFT JOIN task_tags tt ON t.id = tt.task_id LEFT JOIN tags tag ON tt.tag_id = tag.id WHERE 1=1";
    std::vector<std::string> sql_params;
    
    // 添加项目ID过滤
    if (params.project_id) {
        sql += " AND t.project_id = ?";
        sql_params.push_back(std::to_string(params.project_id.value()));
    }
    
    // 添加负责人用户ID过滤
    if (params.assignee_user_id) {
        sql += " AND t.assignee_user_id = ?";
        sql_params.push_back(std::to_string(params.assignee_user_id.value()));
    }
    
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
    
    // 添加关键字过滤
    if (params.keyword) {
        sql += " AND (t.title LIKE ? OR t.description LIKE ?)";
        std::string keyword = "%" + params.keyword.value() + "%";
        sql_params.push_back(keyword);
        sql_params.push_back(keyword);
    }
    
    // 添加标签过滤
    if (params.tag) {
        sql += " AND tag.name = ?";
        sql_params.push_back(params.tag.value());
    }
    
    // 添加分页
    sql += " LIMIT ? OFFSET ?";
    sql_params.push_back(std::to_string(params.page_size));
    sql_params.push_back(std::to_string((params.page - 1) * params.page_size));
    
    return ExecuteQuery<Task>(sql, sql_params, ParseTaskFromStatement);
}

int Database::GetTasksCountByProjectId(int project_id) {
    std::string sql = "SELECT COUNT(*) FROM tasks WHERE project_id = ?";
    std::vector<std::string> params = {std::to_string(project_id)};
    
    auto results = ExecuteQuery<int>(sql, params, [](sqlite3_stmt* stmt) -> int {
        return sqlite3_column_int(stmt, 0);
    });
    
    if (results.empty()) {
        return 0;
    }
    
    return results[0];
}

int Database::GetTasksCountByQueryParams(const TaskQueryParams& params) {
    std::string sql = "SELECT COUNT(DISTINCT t.id) FROM tasks t LEFT JOIN task_tags tt ON t.id = tt.task_id LEFT JOIN tags tag ON tt.tag_id = tag.id WHERE 1=1";
    std::vector<std::string> sql_params;
    
    // 添加项目ID过滤
    if (params.project_id) {
        sql += " AND t.project_id = ?";
        sql_params.push_back(std::to_string(params.project_id.value()));
    }
    
    // 添加负责人用户ID过滤
    if (params.assignee_user_id) {
        sql += " AND t.assignee_user_id = ?";
        sql_params.push_back(std::to_string(params.assignee_user_id.value()));
    }
    
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
    
    // 添加关键字过滤
    if (params.keyword) {
        sql += " AND (t.title LIKE ? OR t.description LIKE ?)";
        std::string keyword = "%" + params.keyword.value() + "%";
        sql_params.push_back(keyword);
        sql_params.push_back(keyword);
    }
    
    // 添加标签过滤
    if (params.tag) {
        sql += " AND tag.name = ?";
        sql_params.push_back(params.tag.value());
    }
    
    auto results = ExecuteQuery<int>(sql, sql_params, [](sqlite3_stmt* stmt) -> int {
        return sqlite3_column_int(stmt, 0);
    });
    
    if (results.empty()) {
        return 0;
    }
    
    return results[0];
}

bool Database::CreateTask(const Task& task) {
    std::string sql = "INSERT INTO tasks (project_id, assignee_user_id, title, description, status, priority, due_date, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";
    
    std::stringstream created_at_ss;
    created_at_ss << std::chrono::system_clock::to_time_t(task.created_at);
    
    std::stringstream updated_at_ss;
    updated_at_ss << std::chrono::system_clock::to_time_t(task.updated_at);
    
    std::vector<std::string> params = {
        std::to_string(task.project_id),
        task.assignee_user_id ? std::to_string(task.assignee_user_id.value()) : "",
        task.title,
        task.description.value_or(""),
        task.status,
        task.priority,
        task.due_date ? std::to_string(std::chrono::system_clock::to_time_t(task.due_date.value())) : "",
        created_at_ss.str(),
        updated_at_ss.str()
    };
    
    return ExecuteNonQuery(sql, params);
}

bool Database::UpdateTask(const Task& task) {
    std::string sql = "UPDATE tasks SET project_id = ?, assignee_user_id = ?, title = ?, description = ?, status = ?, priority = ?, due_date = ?, updated_at = ? WHERE id = ?";
    
    std::stringstream updated_at_ss;
    updated_at_ss << std::chrono::system_clock::to_time_t(task.updated_at);
    
    std::vector<std::string> params = {
        std::to_string(task.project_id),
        task.assignee_user_id ? std::to_string(task.assignee_user_id.value()) : "",
        task.title,
        task.description.value_or(""),
        task.status,
        task.priority,
        task.due_date ? std::to_string(std::chrono::system_clock::to_time_t(task.due_date.value())) : "",
        updated_at_ss.str(),
        std::to_string(task.id)
    };
    
    return ExecuteNonQuery(sql, params);
}

bool Database::DeleteTask(int id) {
    std::string sql = "DELETE FROM tasks WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};
    
    return ExecuteNonQuery(sql, params);
}

// 标签相关操作
std::optional<Tag> Database::GetTagById(int id) {
    std::string sql = "SELECT id, name FROM tags WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};
    
    auto tags = ExecuteQuery<Tag>(sql, params, ParseTagFromStatement);
    
    if (tags.empty()) {
        return std::nullopt;
    }
    
    return tags[0];
}

std::optional<Tag> Database::GetTagByName(const std::string& name) {
    std::string sql = "SELECT id, name FROM tags WHERE name = ?";
    std::vector<std::string> params = {name};
    
    auto tags = ExecuteQuery<Tag>(sql, params, ParseTagFromStatement);
    
    if (tags.empty()) {
        return std::nullopt;
    }
    
    return tags[0];
}

std::vector<Tag> Database::GetAllTags() {
    std::string sql = "SELECT id, name FROM tags";
    std::vector<std::string> params;
    
    return ExecuteQuery<Tag>(sql, params, ParseTagFromStatement);
}

bool Database::CreateTag(const Tag& tag) {
    std::string sql = "INSERT INTO tags (name) VALUES (?)";
    std::vector<std::string> params = {tag.name};
    
    return ExecuteNonQuery(sql, params);
}

bool Database::UpdateTag(const Tag& tag) {
    std::string sql = "UPDATE tags SET name = ? WHERE id = ?";
    std::vector<std::string> params = {tag.name, std::to_string(tag.id)};
    
    return ExecuteNonQuery(sql, params);
}

bool Database::DeleteTag(int id) {
    std::string sql = "DELETE FROM tags WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};
    
    return ExecuteNonQuery(sql, params);
}

// 任务标签关联相关操作
std::vector<Tag> Database::GetTagsByTaskId(int task_id) {
    std::string sql = "SELECT tag.id, tag.name FROM tags tag INNER JOIN task_tags tt ON tag.id = tt.tag_id WHERE tt.task_id = ?";
    std::vector<std::string> params = {std::to_string(task_id)};
    
    return ExecuteQuery<Tag>(sql, params, ParseTagFromStatement);
}

bool Database::AddTagToTask(int task_id, int tag_id) {
    std::string sql = "INSERT OR IGNORE INTO task_tags (task_id, tag_id) VALUES (?, ?)";
    std::vector<std::string> params = {std::to_string(task_id), std::to_string(tag_id)};
    
    return ExecuteNonQuery(sql, params);
}

bool Database::RemoveTagFromTask(int task_id, int tag_id) {
    std::string sql = "DELETE FROM task_tags WHERE task_id = ? AND tag_id = ?";
    std::vector<std::string> params = {std::to_string(task_id), std::to_string(tag_id)};
    
    return ExecuteNonQuery(sql, params);
}

bool Database::RemoveAllTagsFromTask(int task_id) {
    std::string sql = "DELETE FROM task_tags WHERE task_id = ?";
    std::vector<std::string> params = {std::to_string(task_id)};
    
    return ExecuteNonQuery(sql, params);
}

// 审计日志相关操作
std::optional<AuditLog> Database::GetAuditLogById(int id) {
    std::string sql = "SELECT id, user_id, action_type, resource_type, resource_id, created_at, detail FROM audit_logs WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};
    
    auto audit_logs = ExecuteQuery<AuditLog>(sql, params, ParseAuditLogFromStatement);
    
    if (audit_logs.empty()) {
        return std::nullopt;
    }
    
    return audit_logs[0];
}

std::vector<AuditLog> Database::GetAuditLogsByUserId(int user_id, int limit) {
    std::string sql = "SELECT id, user_id, action_type, resource_type, resource_id, created_at, detail FROM audit_logs WHERE user_id = ? ORDER BY created_at DESC LIMIT ?";
    std::vector<std::string> params = {std::to_string(user_id), std::to_string(limit)};
    
    return ExecuteQuery<AuditLog>(sql, params, ParseAuditLogFromStatement);
}

bool Database::CreateAuditLog(const AuditLog& audit_log) {
    std::string sql = "INSERT INTO audit_logs (user_id, action_type, resource_type, resource_id, created_at, detail) VALUES (?, ?, ?, ?, ?, ?)";
    
    std::stringstream created_at_ss;
    created_at_ss << std::chrono::system_clock::to_time_t(audit_log.created_at);
    
    std::vector<std::string> params = {
        std::to_string(audit_log.user_id),
        audit_log.action_type,
        audit_log.resource_type,
        audit_log.resource_id ? std::to_string(audit_log.resource_id.value()) : "",
        created_at_ss.str(),
        audit_log.detail.value_or("")
    };
    
    return ExecuteNonQuery(sql, params);
}

// 统计相关操作
std::optional<ProjectStats> Database::GetProjectStats(int project_id) {
    ProjectStats stats = {0, 0, 0, 0};
    
    // 获取总任务数量
    std::string sql_total = "SELECT COUNT(*) FROM tasks WHERE project_id = ?";
    std::vector<std::string> params_total = {std::to_string(project_id)};
    
    auto results_total = ExecuteQuery<int>(sql_total, params_total, [](sqlite3_stmt* stmt) -> int {
        return sqlite3_column_int(stmt, 0);
    });
    
    if (!results_total.empty()) {
        stats.total_tasks = results_total[0];
    }
    
    // 获取待办任务数量
    std::string sql_todo = "SELECT COUNT(*) FROM tasks WHERE project_id = ? AND status = 'todo'";
    std::vector<std::string> params_todo = {std::to_string(project_id)};
    
    auto results_todo = ExecuteQuery<int>(sql_todo, params_todo, [](sqlite3_stmt* stmt) -> int {
        return sqlite3_column_int(stmt, 0);
    });
    
    if (!results_todo.empty()) {
        stats.todo_tasks = results_todo[0];
    }
    
    // 获取进行中任务数量
    std::string sql_doing = "SELECT COUNT(*) FROM tasks WHERE project_id = ? AND status = 'doing'";
    std::vector<std::string> params_doing = {std::to_string(project_id)};
    
    auto results_doing = ExecuteQuery<int>(sql_doing, params_doing, [](sqlite3_stmt* stmt) -> int {
        return sqlite3_column_int(stmt, 0);
    });
    
    if (!results_doing.empty()) {
        stats.doing_tasks = results_doing[0];
    }
    
    // 获取已完成任务数量
    std::string sql_done = "SELECT COUNT(*) FROM tasks WHERE project_id = ? AND status = 'done'";
    std::vector<std::string> params_done = {std::to_string(project_id)};
    
    auto results_done = ExecuteQuery<int>(sql_done, params_done, [](sqlite3_stmt* stmt) -> int {
        return sqlite3_column_int(stmt, 0);
    });
    
    if (!results_done.empty()) {
        stats.done_tasks = results_done[0];
    }
    
    return stats;
}

std::optional<UserStats> Database::GetUserStats(int user_id) {
    UserStats stats;
    stats.task_stats = {0, 0, 0};
    stats.overdue_tasks = 0;
    stats.recent_tasks = 0;
    stats.total_projects = 0;
    
    // 获取用户的任务状态统计
    std::string sql_task_stats = "SELECT status, COUNT(*) FROM tasks WHERE project_id IN (SELECT id FROM projects WHERE owner_user_id = ?) GROUP BY status";
    std::vector<std::string> params_task_stats = {std::to_string(user_id)};
    
    auto results_task_stats = ExecuteQuery<std::pair<std::string, int>>(sql_task_stats, params_task_stats, [](sqlite3_stmt* stmt) -> std::pair<std::string, int> {
        std::string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int count = sqlite3_column_int(stmt, 1);
        return {status, count};
    });
    
    for (const auto& result : results_task_stats) {
        if (result.first == "todo") {
            stats.task_stats.todo = result.second;
        } else if (result.first == "doing") {
            stats.task_stats.doing = result.second;
        } else if (result.first == "done") {
            stats.task_stats.done = result.second;
        }
    }
    
    // 获取用户的逾期任务数量
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string sql_overdue = "SELECT COUNT(*) FROM tasks WHERE project_id IN (SELECT id FROM projects WHERE owner_user_id = ?) AND due_date <= ? AND status != 'done'";
    std::vector<std::string> params_overdue = {std::to_string(user_id), std::to_string(now)};
    
    auto results_overdue = ExecuteQuery<int>(sql_overdue, params_overdue, [](sqlite3_stmt* stmt) -> int {
        return sqlite3_column_int(stmt, 0);
    });
    
    if (!results_overdue.empty()) {
        stats.overdue_tasks = results_overdue[0];
    }
    
    // 获取用户近7天内新建的任务数量
    std::time_t seven_days_ago = now - (7 * 24 * 60 * 60);
    std::string sql_recent = "SELECT COUNT(*) FROM tasks WHERE project_id IN (SELECT id FROM projects WHERE owner_user_id = ?) AND created_at >= ?";
    std::vector<std::string> params_recent = {std::to_string(user_id), std::to_string(seven_days_ago)};
    
    auto results_recent = ExecuteQuery<int>(sql_recent, params_recent, [](sqlite3_stmt* stmt) -> int {
        return sqlite3_column_int(stmt, 0);
    });
    
    if (!results_recent.empty()) {
        stats.recent_tasks = results_recent[0];
    }
    
    // 获取用户的总项目数量
    std::string sql_total_projects = "SELECT COUNT(*) FROM projects WHERE owner_user_id = ?";
    std::vector<std::string> params_total_projects = {std::to_string(user_id)};
    
    auto results_total_projects = ExecuteQuery<int>(sql_total_projects, params_total_projects, [](sqlite3_stmt* stmt) -> int {
        return sqlite3_column_int(stmt, 0);
    });
    
    if (!results_total_projects.empty()) {
        stats.total_projects = results_total_projects[0];
    }
    
    return stats;
}

// 私有方法实现
template<typename T>
std::vector<T> Database::ExecuteQuery(const std::string& sql, const std::vector<std::string>& params, T(*callback)(sqlite3_stmt*)) {
    std::vector<T> results;
    
    auto conn = connection_pool_->GetConnection();
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn.get(), sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::string error_message = sqlite3_errmsg(conn.get());
        sqlite3_finalize(stmt);
        connection_pool_->ReleaseConnection(conn);
        throw DatabaseException("Failed to prepare SQL statement: " + error_message);
    }
    
    // 绑定参数
    for (int i = 0; i < params.size(); ++i) {
        rc = sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_TRANSIENT);
        
        if (rc != SQLITE_OK) {
            std::string error_message = sqlite3_errmsg(conn.get());
            sqlite3_finalize(stmt);
            connection_pool_->ReleaseConnection(conn);
            throw DatabaseException("Failed to bind parameter: " + error_message);
        }
    }
    
    // 执行查询
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        results.push_back(callback(stmt));
    }
    
    if (rc != SQLITE_DONE) {
        std::string error_message = sqlite3_errmsg(conn.get());
        sqlite3_finalize(stmt);
        connection_pool_->ReleaseConnection(conn);
        throw DatabaseException("Failed to execute SQL query: " + error_message);
    }
    
    // 清理资源
    sqlite3_finalize(stmt);
    connection_pool_->ReleaseConnection(conn);
    
    return results;
}

bool Database::ExecuteNonQuery(const std::string& sql, const std::vector<std::string>& params) {
    auto conn = connection_pool_->GetConnection();
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn.get(), sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::string error_message = sqlite3_errmsg(conn.get());
        sqlite3_finalize(stmt);
        connection_pool_->ReleaseConnection(conn);
        throw DatabaseException("Failed to prepare SQL statement: " + error_message);
    }
    
    // 绑定参数
    for (int i = 0; i < params.size(); ++i) {
        rc = sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_TRANSIENT);
        
        if (rc != SQLITE_OK) {
            std::string error_message = sqlite3_errmsg(conn.get());
            sqlite3_finalize(stmt);
            connection_pool_->ReleaseConnection(conn);
            throw DatabaseException("Failed to bind parameter: " + error_message);
        }
    }
    
    // 执行非查询语句
    rc = sqlite3_step(stmt);
    
    if (rc != SQLITE_DONE) {
        std::string error_message = sqlite3_errmsg(conn.get());
        sqlite3_finalize(stmt);
        connection_pool_->ReleaseConnection(conn);
        throw DatabaseException("Failed to execute SQL non-query: " + error_message);
    }
    
    // 清理资源
    sqlite3_finalize(stmt);
    connection_pool_->ReleaseConnection(conn);
    
    return true;
}

User Database::ParseUserFromStatement(sqlite3_stmt* stmt) {
    User user;
    user.id = sqlite3_column_int(stmt, 0);
    user.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    user.password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    
    std::time_t created_at_time_t = sqlite3_column_int64(stmt, 4);
    user.created_at = std::chrono::system_clock::from_time_t(created_at_time_t);
    
    return user;
}

Project Database::ParseProjectFromStatement(sqlite3_stmt* stmt) {
    Project project;
    project.id = sqlite3_column_int(stmt, 0);
    project.owner_user_id = sqlite3_column_int(stmt, 1);
    project.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    
    if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) {
        project.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    } else {
        project.description = std::nullopt;
    }
    
    std::time_t created_at_time_t = sqlite3_column_int64(stmt, 4);
    project.created_at = std::chrono::system_clock::from_time_t(created_at_time_t);
    
    return project;
}

Task Database::ParseTaskFromStatement(sqlite3_stmt* stmt) {
    Task task;
    task.id = sqlite3_column_int(stmt, 0);
    task.project_id = sqlite3_column_int(stmt, 1);
    
    if (sqlite3_column_type(stmt, 2) != SQLITE_NULL) {
        task.assignee_user_id = sqlite3_column_int(stmt, 2);
    } else {
        task.assignee_user_id = std::nullopt;
    }
    
    task.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    
    if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) {
        task.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    } else {
        task.description = std::nullopt;
    }
    
    task.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    task.priority = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    
    if (sqlite3_column_type(stmt, 7) != SQLITE_NULL) {
        std::time_t due_date_time_t = sqlite3_column_int64(stmt, 7);
        task.due_date = std::chrono::system_clock::from_time_t(due_date_time_t);
    } else {
        task.due_date = std::nullopt;
    }
    
    std::time_t created_at_time_t = sqlite3_column_int64(stmt, 8);
    task.created_at = std::chrono::system_clock::from_time_t(created_at_time_t);
    
    std::time_t updated_at_time_t = sqlite3_column_int64(stmt, 9);
    task.updated_at = std::chrono::system_clock::from_time_t(updated_at_time_t);
    
    return task;
}

Tag Database::ParseTagFromStatement(sqlite3_stmt* stmt) {
    Tag tag;
    tag.id = sqlite3_column_int(stmt, 0);
    tag.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    
    return tag;
}

AuditLog Database::ParseAuditLogFromStatement(sqlite3_stmt* stmt) {
    AuditLog audit_log;
    audit_log.id = sqlite3_column_int(stmt, 0);
    audit_log.user_id = sqlite3_column_int(stmt, 1);
    audit_log.action_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    audit_log.resource_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    
    if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) {
        audit_log.resource_id = sqlite3_column_int(stmt, 4);
    } else {
        audit_log.resource_id = std::nullopt;
    }
    
    std::time_t created_at_time_t = sqlite3_column_int64(stmt, 5);
    audit_log.created_at = std::chrono::system_clock::from_time_t(created_at_time_t);
    
    if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) {
        audit_log.detail = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    } else {
        audit_log.detail = std::nullopt;
    }
    
    return audit_log;
}