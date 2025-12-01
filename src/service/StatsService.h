#ifndef STATS_SERVICE_H
#define STATS_SERVICE_H

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <optional>
#include <nlohmann/json.hpp>
#include "../model/Movie.h"
#include "../repository/WatchRecordRepository.h"
#include "../repository/MovieRepository.h"
#include "../utils/LruCache.h"

namespace service {

// 用户统计信息结构体
struct UserStats {
    int total_movies_watched;  // 总共看过的影片数量
    int total_watch_duration;   // 总观看时长（分钟）
    int last_30_days_count;    // 最近 30 天的观影次数
    int last_30_days_duration; // 最近 30 天的观看时长（分钟）
    std::map<std::string, int> top_types; // 按类型分组的观看时长 Top3
    double average_rating;      // 平均评分
};

class StatsService {
public:
    StatsService()
        : watch_record_repository_(std::make_shared<repository::WatchRecordRepository>())
        , movie_repository_(std::make_shared<repository::MovieRepository>()) {}
    
    StatsService(
        std::shared_ptr<repository::WatchRecordRepository> watch_record_repository,
        std::shared_ptr<repository::MovieRepository> movie_repository)
        : watch_record_repository_(std::move(watch_record_repository))
        , movie_repository_(std::move(movie_repository)) {}
    
    ~StatsService() = default;

    // 获取用户统计信息
    std::optional<UserStats> getUserStats(int user_id);

    // 基于用户历史记录生成推荐影片列表
    std::vector<std::shared_ptr<model::Movie>> getRecommendations(int user_id, int limit = 10);

    // 将用户统计信息转换为 JSON 格式
    nlohmann::json statsToJson(const UserStats& stats);

private:
    std::shared_ptr<repository::WatchRecordRepository> watch_record_repository_;
    std::shared_ptr<repository::MovieRepository> movie_repository_;

    // 计算用户统计信息的核心逻辑
    std::optional<UserStats> calculateUserStats(int user_id);

    // 获取用户最常观看的类型
    std::vector<std::string> getUserTopTypes(int user_id, int limit = 3);

    // 从指定类型中选择用户未看过的影片
    std::vector<std::shared_ptr<model::Movie>> getUnwatchedMoviesByType(int user_id, const std::string& type, int limit);

    StatsService(const StatsService&) = delete;
    StatsService& operator=(const StatsService&) = delete;
};

} // namespace service

#endif // STATS_SERVICE_H
