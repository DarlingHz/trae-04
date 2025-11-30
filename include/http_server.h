// HTTP服务器头文件

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <boost/asio.hpp>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

#include "database.h"
#include "auth_service.h"
#include "task_service.h"
#include "project_service.h"
#include "stats_service.h"
#include "audit_log_service.h"

// HTTP请求结构
struct HttpRequest {
    std::string method;  // HTTP方法（GET、POST、PUT、DELETE、PATCH等）
    std::string path;  // 请求路径
    std::string version;  // HTTP版本
    std::unordered_map<std::string, std::string> headers;  // 请求头
    std::unordered_map<std::string, std::string> query_params;  // 查询参数
    std::string body;  // 请求体
};

// HTTP响应结构
struct HttpResponse {
    int status_code;  // 响应状态码
    std::string status_message;  // 响应状态消息
    std::unordered_map<std::string, std::string> headers;  // 响应头
    std::string body;  // 响应体
    
    // 构造函数
    HttpResponse() : status_code(200), status_message("OK") {
        headers["Content-Type"] = "application/json"; 
    }
};

// HTTP服务器类
class HttpServer {
public:
    // 构造函数
    HttpServer(boost::asio::io_context& io_context, int port,
               std::shared_ptr<Database> database,
               std::shared_ptr<AuthService> auth_service,
               std::shared_ptr<TaskService> task_service,
               std::shared_ptr<ProjectService> project_service,
               std::shared_ptr<StatsService> stats_service,
               std::shared_ptr<AuditLogService> audit_log_service);
    
    // 析构函数
    ~HttpServer();
    
    // 启动服务器
    void Start();
    
    // 停止服务器
    void Stop();
    
private:
    // 处理新连接
    void Accept();
    
    // 处理HTTP请求
    void HandleRequest(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    
    // 处理HTTP请求并返回响应
    HttpResponse HandleRequest(const HttpRequest& request);
    
    // 解析HTTP请求
    std::optional<HttpRequest> ParseRequest(const std::string& request_data);
    std::optional<HttpRequest> ParseHttpRequest(std::shared_ptr<boost::asio::streambuf> buffer, std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    
    // 序列化HTTP响应
    std::string SerializeResponse(const HttpResponse& response);
    
    // 发送HTTP响应
    void SendHttpResponse(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const HttpResponse& response);
    
    // 处理用户注册请求
    HttpResponse HandleUserRegister(const HttpRequest& request);
    
    // 处理用户登录请求
    HttpResponse HandleUserLogin(const HttpRequest& request);
    
    // 处理创建项目请求
    HttpResponse HandleCreateProject(const HttpRequest& request);
    
    // 处理获取项目列表请求
    HttpResponse HandleGetProjects(const HttpRequest& request);
    
    // 处理获取项目详情请求
    HttpResponse HandleGetProject(const HttpRequest& request, int project_id);
    
    // 处理更新项目请求
    HttpResponse HandleUpdateProject(const HttpRequest& request);
    
    // 处理删除项目请求
    HttpResponse HandleDeleteProject(const HttpRequest& request);
    
    // 处理创建任务请求
    HttpResponse HandleCreateTask(const HttpRequest& request, int project_id);
    
    // 处理获取任务列表请求
    HttpResponse HandleGetTasks(const HttpRequest& request);
    
    // 处理根据项目ID获取任务列表请求
    HttpResponse HandleGetTasksByProject(const HttpRequest& request, int project_id);
    
    // 处理获取任务详情请求
    HttpResponse HandleGetTask(const HttpRequest& request, int task_id);
    
    // 处理更新任务请求
    HttpResponse HandleUpdateTask(const HttpRequest& request, int task_id);
    
    // 处理删除任务请求
    HttpResponse HandleDeleteTask(const HttpRequest& request, int task_id);
    
    // 处理搜索任务请求
    HttpResponse HandleSearchTasks(const HttpRequest& request);
    
    // 处理获取统计概览请求
    HttpResponse HandleGetStatsOverview(const HttpRequest& request);
    
    // 处理获取审计日志请求
    HttpResponse HandleGetAuditLogs(const HttpRequest& request);
    
    // 从请求头中获取访问令牌
    std::optional<std::string> GetAccessTokenFromRequest(const HttpRequest& request);
    
    // 验证用户身份
    std::optional<User> AuthenticateUser(const HttpRequest& request);
    
    // 创建成功响应
    HttpResponse CreateSuccessResponse(const std::string& data = "{}");
    
    // 创建失败响应
    HttpResponse CreateErrorResponse(int code, const std::string& message);
    
    boost::asio::io_context& io_context_;  // IO上下文
    boost::asio::ip::tcp::acceptor acceptor_;  //  acceptor
    int port_;  // 服务器端口
    std::size_t thread_pool_size_;  // 线程池大小
    
    // 服务对象
    std::shared_ptr<Database> database_;
    std::shared_ptr<AuthService> auth_service_;
    std::shared_ptr<TaskService> task_service_;
    std::shared_ptr<ProjectService> project_service_;
    std::shared_ptr<StatsService> stats_service_;
    std::shared_ptr<AuditLogService> audit_log_service_;
};

// HTTP服务器异常类
class HttpServerException : public std::exception {
public:
    explicit HttpServerException(const std::string& message) : message_(message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
    
private:
    std::string message_;
};

#endif  // HTTP_SERVER_H