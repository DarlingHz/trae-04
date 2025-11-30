#include "http/handler.h"
#include "utils/logger.h"
#include "utils/time.h"
#include <sstream>

namespace http {

Handler::Handler() : service_(1000) {
}

Handler::Handler(size_t cacheSize) : service_(cacheSize) {
}

void Handler::registerRoutes(httplib::Server& server) {
    // API路由
    server.Post("/api/v1/shorten", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleCreateShortLink(req, res);
    });
    
    server.Get(R"(/api/v1/links/(\d+)/stats)", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetShortLinkStats(req, res);
    });
    
    server.Post(R"(/api/v1/links/(\d+)/disable)", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleDisableShortLink(req, res);
    });
    
    // 短链接重定向路由
    server.Get(R"(/s/(\w+))", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleResolveShortLink(req, res);
    });
}

// 创建短链接处理函数
void Handler::handleCreateShortLink(const httplib::Request& req, httplib::Response& res) {
    try {
        // 解析请求体中的JSON数据
        auto jsonBody = json::parse(req.body);
        
        // 验证请求参数
        if (!jsonBody.contains("long_url")) {
            handleError(res, 400, "Missing required parameter: long_url");
            return;
        }
        
        std::string longUrl = jsonBody["long_url"];
        std::string customCode = jsonBody.contains("custom_code") ? jsonBody["custom_code"] : "";
        std::string alias = jsonBody.contains("alias") ? jsonBody["alias"] : "";
        
        // 调用短链接服务创建短链接
        service::ShortLinkService::CreateShortLinkRequest request;
        request.long_url = longUrl;
        request.custom_alias = customCode;
        
        auto response = service_.createShortLink(request);
        
        // 构造响应
        json responseBody;
        responseBody["id"] = response.link.id;
        responseBody["short_code"] = response.link.short_code;
        responseBody["short_url"] = "http://localhost:8080/s/" + response.link.short_code;
        responseBody["long_url"] = response.link.long_url;
        responseBody["alias"] = response.link.custom_alias;
        responseBody["created_at"] = response.link.create_time;
        responseBody["expires_at"] = response.link.expire_time;
        
        res.status = 201;
        res.set_content(responseBody.dump(), "application/json");
    } catch (const json::parse_error& e) {
        handleError(res, 400, "Invalid JSON format");
    } catch (const std::exception& e) {
        handleError(res, 500, e.what());
    }
}

// 解析短链接处理函数
void Handler::handleResolveShortLink(const httplib::Request& req, httplib::Response& res) {
    try {
        // 获取短码
        auto shortCode = req.matches[1];
        
        // 调用短链接服务解析短链接
        service::ShortLinkService::ResolveShortLinkRequest request;
        request.short_code = shortCode;
        request.ip = req.remote_addr;
        request.user_agent = req.get_header_value("User-Agent");
        
        auto response = service_.resolveShortLink(request);
        
        // 重定向到长链接
        res.status = 302;
        res.set_header("Location", response.long_url);
    } catch (const std::exception& e) {
        handleError(res, 404, "Short link not found");
    }
}

// 获取短链接统计信息处理函数
void Handler::handleGetShortLinkStats(const httplib::Request& req, httplib::Response& res) {
    try {
        // 获取短链接ID
        auto id = std::stoul(req.matches[1]);
        
        // 调用短链接服务获取统计信息
        service::ShortLinkService::GetShortLinkStatsRequest request;
        request.link_id = id;
        
        auto response = service_.getShortLinkStats(request);
        
        // 构造响应
        json responseBody;
        responseBody["id"] = response.stats.link.id;
        responseBody["short_code"] = response.stats.link.short_code;
        responseBody["long_url"] = response.stats.link.long_url;
        responseBody["alias"] = response.stats.link.custom_alias;
        responseBody["created_at"] = response.stats.link.create_time;
        responseBody["expires_at"] = response.stats.link.expire_time;
        responseBody["is_disabled"] = !response.stats.link.is_enabled;
        responseBody["visit_count"] = response.stats.link.visit_count;
        
        // 构造访问日志数组
        json visitLogs;
        for (const auto& log : response.stats.recent_visits) {
            json logItem;
            logItem["id"] = log.id;
            logItem["shortlink_id"] = log.link_id;
            logItem["ip_address"] = log.ip;
            logItem["user_agent"] = log.user_agent;
            logItem["visit_time"] = log.visit_time;
            visitLogs.push_back(logItem);
        }
        
        responseBody["recent_visits"] = visitLogs;
        
        res.status = 200;
        res.set_content(responseBody.dump(), "application/json");
    } catch (const std::invalid_argument& e) {
        handleError(res, 400, "Invalid short link ID");
    } catch (const std::exception& e) {
        handleError(res, 500, e.what());
    }
}

// 禁用短链接处理函数
void Handler::handleDisableShortLink(const httplib::Request& req, httplib::Response& res) {
    try {
        // 获取短链接ID
        auto id = std::stoul(req.matches[1]);
        
        // 调用短链接服务禁用短链接
        service::ShortLinkService::DisableShortLinkRequest request;
        request.link_id = id;
        
        service_.disableShortLink(request);
        
        // 构造响应
        res.status = 204;
    } catch (const std::invalid_argument& e) {
        handleError(res, 400, "Invalid short link ID");
    } catch (const std::exception& e) {
        handleError(res, 500, e.what());
    }
}

// 错误处理函数
void Handler::handleError(httplib::Response& res, int statusCode, const std::string& errorMessage) {
    json responseBody;
    responseBody["error"] = errorMessage;
    
    res.status = statusCode;
    res.set_content(responseBody.dump(), "application/json");
}

} // namespace http