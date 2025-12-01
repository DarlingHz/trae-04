#include "StatsController.h"
#include "../utils/Logger.h"
#include "../utils/TimeUtils.h"
#include <string>
#include <nlohmann/json.hpp>

namespace controller {

StatsController::StatsController(std::shared_ptr<service::StatsService> stats_service)
    : stats_service_(std::move(stats_service)) {
}

void StatsController::getUserStatsSummary(const httplib::Request& req, httplib::Response& res) {
    try {
        // 从 URL 参数中获取用户 ID
        auto it = req.params.find("id");
        if (it == req.params.end()) {
            LOG_ERROR("Invalid request: user ID is required");
            auto response = createResponse(400, "Invalid request: user ID is required");
            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }
        
        int user_id = std::stoi(it->second);
        
        // 调用服务层获取用户统计信息
        auto stats = stats_service_->getUserStats(user_id);
        
        if (stats) {
            LOG_INFO("User stats retrieved successfully for user: " + std::to_string(user_id));
            
            // 将统计信息转换为 JSON
            auto stats_json = stats_service_->statsToJson(*stats);
            
            auto response = createResponse(0, "ok", stats_json);
            res.set_content(response.dump(), "application/json");
            res.status = 200;
        } else {
            LOG_ERROR("Failed to retrieve user stats for user: " + std::to_string(user_id));
            auto response = createResponse(404, "User not found or no stats available");
            res.set_content(response.dump(), "application/json");
            res.status = 404;
        }
    } catch (const std::invalid_argument& e) {
        LOG_ERROR("Invalid user ID: " + std::string(e.what()));
        auto response = createResponse(400, "Invalid user ID");
        res.set_content(response.dump(), "application/json");
        res.status = 400;
    } catch (const std::exception& e) {
        LOG_ERROR("Unexpected error: " + std::string(e.what()));
        auto response = createResponse(500, "Internal server error");
        res.set_content(response.dump(), "application/json");
        res.status = 500;
    }
}

void StatsController::getUserRecommendations(const httplib::Request& req, httplib::Response& res) {
    try {
        // 从 URL 参数中获取用户 ID
        auto it = req.params.find("id");
        if (it == req.params.end()) {
            LOG_ERROR("Invalid request: user ID is required");
            auto response = createResponse(400, "Invalid request: user ID is required");
            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }
        
        int user_id = std::stoi(it->second);
        
        // 从查询参数中获取推荐数量限制
        std::string limit_str = req.get_param_value("limit");
        int limit = (limit_str.empty()) ? 10 : std::stoi(limit_str);
        
        // 调用服务层获取用户推荐列表
        auto recommendations = stats_service_->getRecommendations(user_id, limit);
        
        LOG_INFO("Generated " + std::to_string(recommendations.size()) + " recommendations for user: " + std::to_string(user_id));
        
        // 构造响应数据
        nlohmann::json recommendations_json = nlohmann::json::array();
        for (const auto& movie : recommendations) {
            nlohmann::json movie_json;
            movie_json["id"] = movie->getId();
            movie_json["title"] = movie->getTitle();
            movie_json["type"] = movie->getType();
            movie_json["duration"] = movie->getDuration();
            movie_json["status"] = movie->getStatus();
            movie_json["created_at"] = utils::TimeUtils::timePointToIsoString(movie->getCreatedAt());
            recommendations_json.push_back(movie_json);
        }
        
        auto response = createResponse(0, "ok", recommendations_json);
        res.set_content(response.dump(), "application/json");
        res.status = 200;
    } catch (const std::invalid_argument& e) {
        LOG_ERROR("Invalid parameter: " + std::string(e.what()));
        auto response = createResponse(400, "Invalid parameter");
        res.set_content(response.dump(), "application/json");
        res.status = 400;
    } catch (const std::exception& e) {
        LOG_ERROR("Unexpected error: " + std::string(e.what()));
        auto response = createResponse(500, "Internal server error");
        res.set_content(response.dump(), "application/json");
        res.status = 500;
    }
}

nlohmann::json StatsController::createResponse(int code, const std::string& message, const nlohmann::json& data) {
    nlohmann::json response;
    response["code"] = code;
    response["message"] = message;
    response["data"] = data;
    return response;
}

} // namespace controller
