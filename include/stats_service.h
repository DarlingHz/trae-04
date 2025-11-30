// 统计服务头文件

#ifndef STATS_SERVICE_H
#define STATS_SERVICE_H

#include <memory>

#include "models.h"
#include "database.h"
#include "auth_service.h"

// 统计服务类
class StatsService {
public:
    // 构造函数
    StatsService(std::shared_ptr<Database> database, std::shared_ptr<AuthService> auth_service);
    
    // 析构函数
    ~StatsService();
    
    // 获取用户统计概览
    std::optional<UserStats> GetUserStatsOverview(int user_id);
    
    // 获取用户的任务状态统计
    std::optional<TaskStats> GetUserTaskStats(int user_id);
    
    // 获取用户的逾期任务数量
    int GetUserOverdueTasksCount(int user_id);
    
    // 获取用户近7天内新建的任务数量
    int GetUserRecentTasksCount(int user_id);
    
    // 获取用户的总项目数量
    int GetUserTotalProjectsCount(int user_id);
    
private:
    std::shared_ptr<Database> database_;  // 数据库访问对象
    std::shared_ptr<AuthService> auth_service_;  // 认证服务对象
};

// 统计服务异常类
class StatsServiceException : public std::exception {
public:
    explicit StatsServiceException(const std::string& message) : message_(message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
    
private:
    std::string message_;
};

#endif  // STATS_SERVICE_H