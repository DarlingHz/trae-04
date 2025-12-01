#include "UserController.h"
#include "../utils/Logger.h"
#include "../utils/TimeUtils.h"
#include <string>
#include <nlohmann/json.hpp>

namespace controller {

UserController::UserController(std::shared_ptr<service::UserService> user_service)
    : user_service_(std::move(user_service)) {
}

void UserController::createUser(const httplib::Request& req, httplib::Response& res) {
    try {
        // 解析请求体中的 JSON 数据
        auto json_body = nlohmann::json::parse(req.body);
        
        // 验证请求数据
        if (!json_body.contains("nickname") || !json_body["nickname"].is_string()) {
            LOG_ERROR("Invalid request: nickname is required and must be a string");
            auto response = createResponse(400, "Invalid request: nickname is required and must be a string");
            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }
        
        std::string nickname = json_body["nickname"];
        
        // 调用服务层创建用户
        auto user = user_service_->createUser(nickname);
        
        if (user) {
            LOG_INFO("User created successfully: " + std::to_string(user->getId()));
            
            // 构造响应数据
            nlohmann::json user_json;
            user_json["id"] = user->getId();
            user_json["nickname"] = user->getNickname();
            user_json["created_at"] = utils::TimeUtils::timePointToIsoString(user->getCreatedAt());
            
            auto response = createResponse(0, "ok", user_json);
            res.set_content(response.dump(), "application/json");
            res.status = 201;
        } else {
            LOG_ERROR("Failed to create user");
            auto response = createResponse(500, "Failed to create user");
            res.set_content(response.dump(), "application/json");
            res.status = 500;
        }
    } catch (const nlohmann::json::parse_error& e) {
        LOG_ERROR("JSON parse error: " + std::string(e.what()));
        auto response = createResponse(400, "Invalid JSON format");
        res.set_content(response.dump(), "application/json");
        res.status = 400;
    } catch (const std::exception& e) {
        LOG_ERROR("Unexpected error: " + std::string(e.what()));
        auto response = createResponse(500, "Internal server error");
        res.set_content(response.dump(), "application/json");
        res.status = 500;
    }
}

void UserController::getUserById(const httplib::Request& req, httplib::Response& res) {
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
        
        // 调用服务层获取用户信息
        auto user = user_service_->getUserById(user_id);
        
        if (user) {
            LOG_INFO("User retrieved successfully: " + std::to_string(user->getId()));
            
            // 构造响应数据
            nlohmann::json user_json;
            user_json["id"] = user->getId();
            user_json["nickname"] = user->getNickname();
            user_json["created_at"] = utils::TimeUtils::timePointToIsoString(user->getCreatedAt());
            
            auto response = createResponse(0, "ok", user_json);
            res.set_content(response.dump(), "application/json");
            res.status = 200;
        } else {
            LOG_ERROR("User not found: " + std::to_string(user_id));
            auto response = createResponse(404, "User not found");
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

nlohmann::json UserController::createResponse(int code, const std::string& message, const nlohmann::json& data) {
    nlohmann::json response;
    response["code"] = code;
    response["message"] = message;
    response["data"] = data;
    return response;
}

} // namespace controller
