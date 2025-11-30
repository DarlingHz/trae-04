#include "common/logger.h"
#include "common/config.h"
#include "common/error.h"
#include "models/db_pool.h"
#include "models/user.h"
#include "models/problem.h"
#include "models/record.h"
#include "auth/auth_service.h"
#include "controllers/user_controller.h"
#include "controllers/problem_controller.h"
#include "controllers/record_controller.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <sstream>

namespace http {

// HTTP请求处理器接口
class RequestHandler {
public:
    virtual controllers::HttpResponse handle(const controllers::HttpRequest& request) = 0;
    virtual ~RequestHandler() = default;
};

// 路由器类，用于分发请求
class Router {
public:
    void registerRoute(const std::string& pathPattern, RequestHandler* handler) {
        routes_[pathPattern] = handler;
    }
    
    controllers::HttpResponse routeRequest(const controllers::HttpRequest& request) {
        std::string path = request.path;
        
        // 匹配具体的路由模式
        for (const auto& [pattern, handler] : routes_) {
            // 简单的路由匹配，支持精确匹配
            if (path == pattern) {
                return handler->handle(request);
            }
            
            // 支持通配符路径
            if (pattern.find("*") != std::string::npos) {
                std::string prefix = pattern.substr(0, pattern.find("*"));
                if (path.find(prefix) == 0) {
                    return handler->handle(request);
                }
            }
        }
        
        // 未找到匹配的路由
        controllers::HttpResponse response;
        response.statusCode = 404;
        response.headers["Content-Type"] = "application/json";
        response.body = R"({
            "error": {
                "code": "NOT_FOUND",
                "message": "API endpoint not found"
            }
        })";
        return response;
    }
    
private:
    std::map<std::string, RequestHandler*> routes_;
};

// 控制器适配器，将控制器适配到请求处理器接口
class ControllerAdapter : public RequestHandler {
public:
    ControllerAdapter(controllers::BaseController* controller) : controller_(controller) {}
    
    controllers::HttpResponse handle(const controllers::HttpRequest& request) override {
        return controller_->handleRequest(request);
    }
    
private:
    controllers::BaseController* controller_;
};

// HTTP服务器类
class HttpServer {
public:
    HttpServer(int port, Router* router) : port_(port), router_(router) {}
    
    bool start() {
        // 创建套接字
        serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket_ < 0) {
            common::g_logger.error("Failed to create socket: %s", std::strerror(errno));
            return false;
        }
        
        // 设置SO_REUSEADDR选项
        int opt = 1;
        if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            common::g_logger.error("Failed to set socket options: %s", std::strerror(errno));
            return false;
        }
        
        // 绑定地址
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port_);
        
        if (bind(serverSocket_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            common::g_logger.error("Failed to bind socket: %s", std::strerror(errno));
            return false;
        }
        
        // 开始监听
        if (listen(serverSocket_, 10) < 0) {
            common::g_logger.error("Failed to listen on socket: %s", std::strerror(errno));
            return false;
        }
        
        common::g_logger.info("HTTP server started on port %d", port_);
        
        return true;
    }
    
    void run() {
        if (!isRunning_) {
            isRunning_ = true;
            
            // 主线程处理请求
            while (isRunning_) {
                sockaddr_in clientAddr;
                socklen_t clientAddrLen = sizeof(clientAddr);
                
                // 接受连接
                int clientSocket = accept(serverSocket_, (struct sockaddr*)&clientAddr, &clientAddrLen);
                if (clientSocket < 0) {
                    if (errno == EINTR) {
                        continue; // 被信号中断，继续处理
                    }
                    common::g_logger.error("Failed to accept connection: %s", std::strerror(errno));
                    continue;
                }
                
                // 处理请求（为简化，使用主线程同步处理）
                handleClient(clientSocket);
            }
        }
    }
    
    void stop() {
        isRunning_ = false;
        
        // 关闭服务器套接字
        if (serverSocket_ > 0) {
            close(serverSocket_);
            serverSocket_ = -1;
        }
        
        common::g_logger.info("HTTP server stopped");
    }
    
private:
    // 处理客户端请求
    void handleClient(int clientSocket) {
        try {
            // 读取请求数据
            char buffer[1024 * 1024]; // 1MB缓冲区
            ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
            
            if (bytesRead <= 0) {
                close(clientSocket);
                return;
            }
            
            buffer[bytesRead] = '\0';
            
            // 解析HTTP请求
            controllers::HttpRequest request = parseHttpRequest(buffer);
            
            // 路由请求
            controllers::HttpResponse response = router_->routeRequest(request);
            
            // 发送响应
            sendResponse(clientSocket, response);
            
        } catch (const std::exception& e) {
            common::g_logger.error("Error handling client request: %s", e.what());
            
            // 发送错误响应
            controllers::HttpResponse errorResponse;
            errorResponse.statusCode = 500;
            errorResponse.headers["Content-Type"] = "application/json";
            errorResponse.body = R"({
                "error": {
                    "code": "INTERNAL_ERROR",
                    "message": "Internal server error"
                }
            })";
            
            sendResponse(clientSocket, errorResponse);
        }
        
        close(clientSocket);
    }
    
    // 解析HTTP请求
    controllers::HttpRequest parseHttpRequest(const char* data) {
        controllers::HttpRequest request;
        std::istringstream stream(data);
        std::string line;
        
        // 解析请求行
        if (std::getline(stream, line)) {
            std::istringstream requestLine(line);
            std::string methodStr, path, httpVersion;
            requestLine >> methodStr >> path >> httpVersion;
            
            // 解析请求方法
            if (methodStr == "GET") request.method = controllers::HttpMethod::GET;
            else if (methodStr == "POST") request.method = controllers::HttpMethod::POST;
            else if (methodStr == "PUT") request.method = controllers::HttpMethod::PUT;
            else if (methodStr == "DELETE") request.method = controllers::HttpMethod::DELETE;
            
            // 解析路径和查询参数
            size_t queryPos = path.find('?');
            if (queryPos != std::string::npos) {
                request.path = path.substr(0, queryPos);
                
                // 解析查询参数
                std::string queryString = path.substr(queryPos + 1);
                size_t paramStart = 0;
                while (paramStart < queryString.length()) {
                    size_t paramEnd = queryString.find('&', paramStart);
                    if (paramEnd == std::string::npos) paramEnd = queryString.length();
                    
                    std::string param = queryString.substr(paramStart, paramEnd - paramStart);
                    size_t equalsPos = param.find('=');
                    if (equalsPos != std::string::npos) {
                        std::string key = param.substr(0, equalsPos);
                        std::string value = param.substr(equalsPos + 1);
                        request.queryParams[key] = value;
                    }
                    
                    paramStart = paramEnd + 1;
                }
            } else {
                request.path = path;
            }
        }
        
        // 解析请求头
        bool headersEnd = false;
        while (std::getline(stream, line) && !headersEnd) {
            // 去除可能的回车符
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            
            // 空行表示请求头结束
            if (line.empty()) {
                headersEnd = true;
                break;
            }
            
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string headerName = line.substr(0, colonPos);
                std::string headerValue = line.substr(colonPos + 2); // 跳过冒号和空格
                request.headers[headerName] = headerValue;
            }
        }
        
        // 读取请求体
        std::string body;
        while (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            body += line + '\n';
        }
        
        // 去除末尾的换行符
        if (!body.empty() && body.back() == '\n') {
            body.pop_back();
        }
        
        request.body = body;
        
        return request;
    }
    
    // 发送HTTP响应
    void sendResponse(int clientSocket, const controllers::HttpResponse& response) {
        std::stringstream responseStream;
        
        // 状态行
        responseStream << "HTTP/1.1 " << response.statusCode << " " 
                      << getStatusMessage(response.statusCode) << "\r\n";
        
        // 响应头
        for (const auto& [name, value] : response.headers) {
            responseStream << name << ": " << value << "\r\n";
        }
        
        // 内容长度
        responseStream << "Content-Length: " << response.body.length() << "\r\n";
        
        // CORS支持
        responseStream << "Access-Control-Allow-Origin: *\r\n";
        responseStream << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
        responseStream << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
        
        // 空行表示响应头结束
        responseStream << "\r\n";
        
        // 响应体
        responseStream << response.body;
        
        // 发送响应
        std::string responseData = responseStream.str();
        send(clientSocket, responseData.c_str(), responseData.length(), 0);
    }
    
    // 获取状态码对应的消息
    std::string getStatusMessage(int statusCode) {
        std::map<int, std::string> statusMessages = {
            {200, "OK"},
            {201, "Created"},
            {400, "Bad Request"},
            {401, "Unauthorized"},
            {403, "Forbidden"},
            {404, "Not Found"},
            {500, "Internal Server Error"}
        };
        
        auto it = statusMessages.find(statusCode);
        if (it != statusMessages.end()) {
            return it->second;
        }
        return "Unknown Status";
    }
    
private:
    int port_;
    Router* router_;
    int serverSocket_ = -1;
    bool isRunning_ = false;
};

} // namespace http

int main() {
    try {
        // 初始化日志
        common::g_logger.setLogLevel(common::LogLevel::INFO);
        common::g_logger.setLogFile("app.log");
        common::g_logger.info("Starting LeetCode Clone Backend Server...");
        
        // 初始化配置
        if (!common::g_config.loadFromFile("config/app.conf")) {
            common::g_logger.warning("Failed to load config file, using default settings");
        }
        
        // 初始化数据库连接池
        std::string dbHost = common::g_config.getString("database.host", "localhost");
        std::string dbPort = common::g_config.getString("database.port", "3306");
        std::string dbUsername = common::g_config.getString("database.username", "root");
        std::string dbPassword = common::g_config.getString("database.password", "");
        std::string dbName = common::g_config.getString("database.name", "online_judge");
        int poolSize = common::g_config.getInt("database.pool_size", 10);
        
        if (!models::initDatabasePool(dbHost, dbPort, dbUsername, dbPassword, dbName, poolSize)) {
            common::g_logger.fatal("Failed to initialize database connection pool");
            return 1;
        }
        common::g_logger.info("Database connection pool initialized");
        
        // 初始化仓库
        models::initUserRepository();
        models::initProblemRepository();
        models::initRecordRepository();
        common::g_logger.info("Repositories initialized");
        
        // 初始化认证服务
        auth::initAuthService();
        common::g_logger.info("Auth service initialized");
        
        // 创建控制器实例
        controllers::UserController userController;
        controllers::ProblemController problemController;
        controllers::RecordController recordController;
        
        // 创建路由器并注册路由
        http::Router router;
        router.registerRoute("/api/v1/auth/register", new http::ControllerAdapter(&userController));
        router.registerRoute("/api/v1/auth/login", new http::ControllerAdapter(&userController));
        router.registerRoute("/api/v1/users/me", new http::ControllerAdapter(&userController));
        
        router.registerRoute("/api/v1/problems", new http::ControllerAdapter(&problemController));
        router.registerRoute("/api/v1/problems*", new http::ControllerAdapter(&problemController));
        
        router.registerRoute("/api/v1/users*", new http::ControllerAdapter(&recordController));
        
        // 创建并启动HTTP服务器
        int port = common::g_config.getInt("server.port", 8000);
        http::HttpServer server(port, &router);
        
        if (!server.start()) {
            common::g_logger.fatal("Failed to start HTTP server");
            return 1;
        }
        
        common::g_logger.info("LeetCode Clone Backend Server is running on port %d", port);
        common::g_logger.info("Press Ctrl+C to stop the server");
        
        // 运行服务器
        server.run();
        
    } catch (const std::exception& e) {
        common::g_logger.fatal("Fatal error: %s", e.what());
        return 1;
    }
    
    return 0;
}
