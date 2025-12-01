#pragma once

#include <cpp-httplib/httplib.h>
#include <memory>
#include <nlohmann/json.hpp>
#include "../service/UserService.h"

namespace controller {

class UserController {
public:
    explicit UserController(std::shared_ptr<service::UserService> user_service);
    
    // 处理 HTTP 请求的方法
    void createUser(const httplib::Request& req, httplib::Response& res);
    void getUserById(const httplib::Request& req, httplib::Response& res);
    
private:
    std::shared_ptr<service::UserService> user_service_;
    
    // 辅助方法
    nlohmann::json createResponse(int code, const std::string& message, const nlohmann::json& data = nlohmann::json::object());
};

} // namespace controller
