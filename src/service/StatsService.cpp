#include "StatsService.h"
#include "../utils/Logger.h"
#include "../utils/TimeUtils.h"
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <nlohmann/json.hpp>

namespace service {

std::optional<UserStats> StatsService::getUserStats(int user_id) {
    if (user_id <= 0) {
        LOG_ERROR("Invalid user ID: " + std::to_string(user_id));
        return std::nullopt;
    }

    // 计算统计信息
    auto stats = calculateUserStats(user_id);
    if (stats) {
        LOG_DEBUG("User stats calculated for user: " + std::to_string(user_id));
    }

    return stats;
}

std::optional<UserStats> StatsService::calculateUserStats(int user_id) {
    // 获取用户的所有观影记录
    auto records = watch_record_repository_->getAllWatchRecordsByUserId(user_id);
    if (records.empty()) {
        LOG_DEBUG("No watch records found for user: " + std::to_string(user_id));
        return std::nullopt;
    }

    UserStats stats;
    stats.total_movies_watched = records.size();
    
    // 计算总观看时长
    stats.total_watch_duration = std::accumulate(
        records.begin(), records.end(), 0,
        [](int sum, const std::shared_ptr<model::WatchRecord>& record) {
            return sum + record->getWatchDuration();
        }
    );

    // 计算最近 30 天的统计数据
    auto now = std::chrono::system_clock::now();
    auto thirty_days_ago = now - std::chrono::days(30);
    
    int last_30_days_count = 0;
    int last_30_days_duration = 0;
    
    // 计算平均评分
    double total_rating = 0.0;
    int rated_count = 0;
    
    // 按类型分组的观看时长
    std::map<std::string, int> type_duration_map;
    
    for (const auto& record : records) {
        // 检查是否在最近 30 天内
        if (record->getStartTime() >= thirty_days_ago) {
            last_30_days_count++;
            last_30_days_duration += record->getWatchDuration();
        }

        // 计算平均评分
        if (record->getRating() > 0.0) {
            total_rating += record->getRating();
            rated_count++;
        }

        // 获取影片类型并更新类型时长映射
        auto movie = movie_repository_->getMovieById(record->getMovieId());
        if (movie) {
            type_duration_map[movie->getType()] += record->getWatchDuration();
        }
    }

    stats.last_30_days_count = last_30_days_count;
    stats.last_30_days_duration = last_30_days_duration;

    // 计算平均评分
    if (rated_count > 0) {
        stats.average_rating = total_rating / rated_count;
    } else {
        stats.average_rating = 0.0;
    }

    // 找到观看时长 Top3 的类型
    std::vector<std::pair<std::string, int>> type_duration_vec(type_duration_map.begin(), type_duration_map.end());
    
    // 按观看时长降序排序
    std::sort(type_duration_vec.begin(), type_duration_vec.end(),
        [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
            return a.second > b.second;
        }
    );

    // 取前 3 个类型
    int top_limit = std::min(3, static_cast<int>(type_duration_vec.size()));
    for (int i = 0; i < top_limit; ++i) {
        stats.top_types[type_duration_vec[i].first] = type_duration_vec[i].second;
    }

    return stats;
}

std::vector<std::shared_ptr<model::Movie>> StatsService::getRecommendations(int user_id, int limit) {
    if (user_id <= 0) {
        LOG_ERROR("Invalid user ID: " + std::to_string(user_id));
        return {};
    }

    if (limit <= 0) {
        LOG_WARN("Invalid limit, using default: 10");
        limit = 10;
    }

    // 推荐逻辑：
    // 1. 获取用户最常观看的类型 Top3
    // 2. 从每个类型中选择用户未看过的影片
    // 3. 合并结果并返回

    std::vector<std::shared_ptr<model::Movie>> recommendations;
    
    // 获取用户最常观看的类型
    auto top_types = getUserTopTypes(user_id, 3);
    
    for (const auto& type : top_types) {
        // 从该类型中选择用户未看过的影片
        auto unwatched_movies = getUnwatchedMoviesByType(user_id, type, limit / top_types.size() + 1);
        
        // 将未看过的影片添加到推荐列表中
        recommendations.insert(recommendations.end(), unwatched_movies.begin(), unwatched_movies.end());
        
        // 如果已经达到推荐数量限制，则停止
        if (recommendations.size() >= static_cast<size_t>(limit)) {
            break;
        }
    }

    // 如果推荐列表不足限制数量，则从其他类型中补充
    if (recommendations.size() < static_cast<size_t>(limit)) {
        auto all_movies = movie_repository_->getAllActiveMovies();
        std::vector<std::shared_ptr<model::Movie>> other_movies;
        
        for (const auto& movie : all_movies) {
            // 检查是否已经在推荐列表中
            bool already_recommended = std::any_of(
                recommendations.begin(), recommendations.end(),
                [&movie](const std::shared_ptr<model::Movie>& recommended_movie) {
                    return recommended_movie->getId() == movie->getId();
                }
            );
            
            if (!already_recommended) {
                other_movies.push_back(movie);
            }
        }
        
        // 添加其他类型的影片，直到达到限制数量
        int remaining = limit - static_cast<int>(recommendations.size());
        for (int i = 0; i < remaining && static_cast<size_t>(i) < other_movies.size(); ++i) {
            recommendations.push_back(other_movies[i]);
        }
    }

    // 如果推荐列表仍然超过限制数量，则截断
    if (recommendations.size() > static_cast<size_t>(limit)) {
        recommendations.resize(static_cast<size_t>(limit));
    }

    LOG_DEBUG("Generated " + std::to_string(recommendations.size()) + " recommendations for user: " + std::to_string(user_id));
    return recommendations;
}

std::vector<std::string> StatsService::getUserTopTypes(int user_id, int limit) {
    if (user_id <= 0) {
        LOG_ERROR("Invalid user ID: " + std::to_string(user_id));
        return {};
    }

    if (limit <= 0) {
        LOG_WARN("Invalid limit, using default: 3");
        limit = 3;
    }

    // 获取用户的所有观影记录
    auto records = watch_record_repository_->getAllWatchRecordsByUserId(user_id);
    if (records.empty()) {
        LOG_DEBUG("No watch records found for user: " + std::to_string(user_id));
        return {};
    }

    // 按类型分组的观看时长
    std::map<std::string, int> type_duration_map;
    
    for (const auto& record : records) {
        // 获取影片类型并更新类型时长映射
        auto movie = movie_repository_->getMovieById(record->getMovieId());
        if (movie) {
            type_duration_map[movie->getType()] += record->getWatchDuration();
        }
    }

    // 将映射转换为向量，以便排序
    std::vector<std::pair<std::string, int>> type_duration_vec(type_duration_map.begin(), type_duration_map.end());
    
    // 按观看时长降序排序
    std::sort(type_duration_vec.begin(), type_duration_vec.end(),
        [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
            return a.second > b.second;
        }
    );

    // 提取前 limit 个类型
    std::vector<std::string> top_types;
    int actual_limit = std::min(limit, static_cast<int>(type_duration_vec.size()));
    for (int i = 0; i < actual_limit; ++i) {
        top_types.push_back(type_duration_vec[i].first);
    }

    return top_types;
}

std::vector<std::shared_ptr<model::Movie>> StatsService::getUnwatchedMoviesByType(int user_id, const std::string& type, int limit) {
    if (user_id <= 0) {
        LOG_ERROR("Invalid user ID: " + std::to_string(user_id));
        return {};
    }

    if (type.empty()) {
        LOG_ERROR("Invalid movie type: empty string");
        return {};
    }

    if (limit <= 0) {
        LOG_WARN("Invalid limit, using default: 5");
        limit = 5;
    }

    // 获取用户已经看过的影片 ID 列表
    auto records = watch_record_repository_->getAllWatchRecordsByUserId(user_id);
    std::vector<int> watched_movie_ids;
    for (const auto& record : records) {
        watched_movie_ids.push_back(record->getMovieId());
    }

    // 获取指定类型的所有未删除影片
    auto all_movies = movie_repository_->getMovies("", type);
    
    // 过滤出用户未看过的影片
    std::vector<std::shared_ptr<model::Movie>> unwatched_movies;
    for (const auto& movie : all_movies) {
        // 检查影片是否已经被用户看过
        bool already_watched = std::find(
            watched_movie_ids.begin(), watched_movie_ids.end(), movie->getId()) != watched_movie_ids.end();
        
        if (!already_watched) {
            unwatched_movies.push_back(movie);
        }
        
        // 如果已经达到限制数量，则停止
        if (unwatched_movies.size() >= static_cast<size_t>(limit)) {
            break;
        }
    }

    return unwatched_movies;
}

nlohmann::json StatsService::statsToJson(const UserStats& stats) {
    nlohmann::json json;
    json["total_movies_watched"] = stats.total_movies_watched;
    json["total_watch_duration"] = stats.total_watch_duration;
    json["last_30_days_count"] = stats.last_30_days_count;
    json["last_30_days_duration"] = stats.last_30_days_duration;
    
    // 将 top_types 转换为 JSON 数组
    nlohmann::json top_types_json = nlohmann::json::array();
    for (const auto& [type, duration] : stats.top_types) {
        nlohmann::json type_json;
        type_json["type"] = type;
        type_json["duration"] = duration;
        top_types_json.push_back(type_json);
    }
    json["top_types"] = top_types_json;
    
    json["average_rating"] = stats.average_rating;
    
    return json;
}

} // namespace service
