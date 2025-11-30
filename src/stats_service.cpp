// 统计服务源文件

#include "stats_service.h"

#include <stdexcept>

// 统计服务实现
StatsService::StatsService(std::shared_ptr<Database> database, std::shared_ptr<AuthService> auth_service) : database_(database), auth_service_(auth_service) {
    if (!database_) {
        throw StatsServiceException("Database instance is null");
    }
    if (!auth_service_) {
        throw StatsServiceException("AuthService instance is null");
    }
}

StatsService::~StatsService() {
}

std::optional<UserStats> StatsService::GetUserStatsOverview(int user_id) {
    // 获取用户统计信息
    return database_->GetUserStats(user_id);
}

std::optional<TaskStats> StatsService::GetUserTaskStats(int user_id) {
    // 获取用户的任务状态统计
    auto user_stats = database_->GetUserStats(user_id);
    if (user_stats) {
        return user_stats->task_stats;
    }
    return std::nullopt;
}

int StatsService::GetUserOverdueTasksCount(int user_id) {
    // 获取用户的逾期任务数量
    auto user_stats = database_->GetUserStats(user_id);
    if (user_stats) {
        return user_stats->overdue_tasks;
    }
    return 0;
}

int StatsService::GetUserRecentTasksCount(int user_id) {
    // 获取用户近7天内新建的任务数量
    auto user_stats = database_->GetUserStats(user_id);
    if (user_stats) {
        return user_stats->recent_tasks;
    }
    return 0;
}

int StatsService::GetUserTotalProjectsCount(int user_id) {
    // 获取用户的总项目数量
    auto user_stats = database_->GetUserStats(user_id);
    if (user_stats) {
        return user_stats->total_projects;
    }
    return 0;
}