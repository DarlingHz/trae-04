#pragma once

#include "service/shortlink_service.h"
#include <httplib.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace http {

class Handler {
public:
    Handler();
    explicit Handler(size_t cacheSize);
    
    // 注册路由
    void registerRoutes(httplib::Server& server);
    
private:
    // 创建短链接处理函数
    void handleCreateShortLink(const httplib::Request& req, httplib::Response& res);
    
    // 解析短链接处理函数
    void handleResolveShortLink(const httplib::Request& req, httplib::Response& res);
    
    // 获取短链接统计信息处理函数
    void handleGetShortLinkStats(const httplib::Request& req, httplib::Response& res);
    
    // 禁用短链接处理函数
    void handleDisableShortLink(const httplib::Request& req, httplib::Response& res);
    
    // 错误处理函数
    void handleError(httplib::Response& res, int statusCode, const std::string& errorMessage);
    
    service::ShortLinkService service_;
};

} // namespace http
