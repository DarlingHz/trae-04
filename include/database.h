// 数据库访问层头文件

#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <mutex>
#include <queue>

#include "models.h"

// 数据库连接池类
class DatabaseConnectionPool {
public:
    // 构造函数
    DatabaseConnectionPool(const std::string& db_path, int pool_size);
    
    // 析构函数
    ~DatabaseConnectionPool();
    
    // 获取数据库连接
    std::shared_ptr<sqlite3> GetConnection();
    
    // 释放数据库连接
    void ReleaseConnection(std::shared_ptr<sqlite3> conn);
    
private:
    std::string db_path_;  // 数据库文件路径
    int pool_size_;  // 连接池大小
    std::queue<std::shared_ptr<sqlite3>> connections_;  // 连接队列
    std::mutex mutex_;  // 互斥锁，用于保护连接队列
};

// 数据库访问类
class Database {
public:
    // 构造函数
    Database(const std::string& db_path, int pool_size = 10);
    
    // 析构函数
    ~Database();
    
    // 用户相关操作
    std::optional<User> GetUserById(int id);
    std::optional<User> GetUserByEmail(const std::string& email);
    bool CreateUser(const User& user);
    bool UpdateUser(const User& user);
    bool DeleteUser(int id);
    
    // 项目相关操作
    std::optional<Project> GetProjectById(int id);
    std::vector<Project> GetProjectsByOwnerUserId(int owner_user_id, int page = 1, int page_size = 10);
    int GetProjectsCountByOwnerUserId(int owner_user_id);
    bool CreateProject(const Project& project);
    bool UpdateProject(const Project& project);
    bool DeleteProject(int id);
    
    // 任务相关操作
    std::optional<Task> GetTaskById(int id);
    std::vector<Task> GetTasksByProjectId(int project_id, int page = 1, int page_size = 10);
    std::vector<Task> GetTasksByQueryParams(const TaskQueryParams& params);
    int GetTasksCountByProjectId(int project_id);
    int GetTasksCountByQueryParams(const TaskQueryParams& params);
    bool CreateTask(const Task& task);
    bool UpdateTask(const Task& task);
    bool DeleteTask(int id);
    
    // 标签相关操作
    std::optional<Tag> GetTagById(int id);
    std::optional<Tag> GetTagByName(const std::string& name);
    std::vector<Tag> GetAllTags();
    bool CreateTag(const Tag& tag);
    bool UpdateTag(const Tag& tag);
    bool DeleteTag(int id);
    
    // 任务标签关联相关操作
    std::vector<Tag> GetTagsByTaskId(int task_id);
    bool AddTagToTask(int task_id, int tag_id);
    bool RemoveTagFromTask(int task_id, int tag_id);
    bool RemoveAllTagsFromTask(int task_id);
    
    // 审计日志相关操作
    std::optional<AuditLog> GetAuditLogById(int id);
    std::vector<AuditLog> GetAuditLogsByUserId(int user_id, int limit = 10);
    bool CreateAuditLog(const AuditLog& audit_log);
    
    // 统计相关操作
    std::optional<ProjectStats> GetProjectStats(int project_id);
    std::optional<UserStats> GetUserStats(int user_id);
    
public:
    // 执行查询语句并返回结果
    template<typename T>
    std::vector<T> ExecuteQuery(const std::string& sql, const std::vector<std::string>& params,
                                  T(*callback)(sqlite3_stmt*));
    
    // 执行非查询语句（INSERT、UPDATE、DELETE等）
    bool ExecuteNonQuery(const std::string& sql, const std::vector<std::string>& params);
    
    // 从SQLite语句中解析用户数据
    static User ParseUserFromStatement(sqlite3_stmt* stmt);
    
    // 从SQLite语句中解析项目数据
    static Project ParseProjectFromStatement(sqlite3_stmt* stmt);
    
    // 从SQLite语句中解析任务数据
    static Task ParseTaskFromStatement(sqlite3_stmt* stmt);
    
    // 从SQLite语句中解析标签数据
    static Tag ParseTagFromStatement(sqlite3_stmt* stmt);
    
    // 从SQLite语句中解析审计日志数据
    static AuditLog ParseAuditLogFromStatement(sqlite3_stmt* stmt);

private:
    
    std::shared_ptr<DatabaseConnectionPool> connection_pool_;  // 数据库连接池
};

// 数据库异常类
class DatabaseException : public std::exception {
public:
    explicit DatabaseException(const std::string& message) : message_(message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
    
private:
    std::string message_;
};

#endif  // DATABASE_H