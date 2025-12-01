#pragma once

#include <cpp-httplib/httplib.h>
#include <memory>
#include <nlohmann/json.hpp>
#include "../service/StatsService.h"

namespace controller {

class StatsController {
public:
    explicit StatsController(std::shared_ptr<service::StatsService> stats_service);
    
    // 处理 HTTP 请求的方法
    void getUserStatsSummary(const httplib::Request& req, httplib::Response& res);
    void getUserRecommendations(const httplib::Request& req, httplib::Response& res);
    
private:
    std::shared_ptr<service::StatsService> stats_service_;
    
    // 辅助方法
    nlohmann::json createResponse(int code, const std::string& message, const nlohmann::json& data = nlohmann::json::object());
};

} // namespace controller
