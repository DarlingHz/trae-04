#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <functional>
#include <map>
#include "database.h"
#include "product.h"
#include "order.h"
#include "stats.h"

// HTTP 请求数据结构
struct HttpRequest {
    std::string method;
    std::string path;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> query_params;
    std::string body;
};

// HTTP 响应数据结构
struct HttpResponse {
    int status_code;
    std::map<std::string, std::string> headers;
    std::string body;
    
    HttpResponse() : status_code(200) {
        headers["Content-Type"] = "application/json";
    }
};

// 路由处理函数类型
typedef std::function<HttpResponse(const HttpRequest&)> RouteHandler;

// HTTP 服务器类
class HttpServer {
public:
    HttpServer(int port, Database& db);
    ~HttpServer();
    
    // 启动服务器
    void start();
    
    // 停止服务器
    void stop();
    
private:
    int port_;
    int server_fd_;
    bool is_running_;
    Database& db_;
    
    // 服务实例
    ProductService product_service_;
    OrderService order_service_;
    StatsService stats_service_;
    
    // 路由表
    std::map<std::string, std::map<std::string, RouteHandler>> routes_;
    
    // 初始化路由
    void init_routes();
    
    // 处理客户端连接
    void handle_client(int client_fd);
    
    // 解析 HTTP 请求
    HttpRequest parse_request(const std::string& request_str);
    
    // 构建 HTTP 响应字符串
    std::string build_response(const HttpResponse& response);
    
    // 商品相关路由处理
    HttpResponse handle_create_product(const HttpRequest& request);
    HttpResponse handle_get_products(const HttpRequest& request);
    HttpResponse handle_get_product(const HttpRequest& request);
    HttpResponse handle_update_product(const HttpRequest& request);
    HttpResponse handle_adjust_stock(const HttpRequest& request);
    
    // 订单相关路由处理
    HttpResponse handle_create_order(const HttpRequest& request);
    HttpResponse handle_get_order(const HttpRequest& request);
    HttpResponse handle_get_orders(const HttpRequest& request);
    HttpResponse handle_update_order_status(const HttpRequest& request);
    
    // 统计相关路由处理
    HttpResponse handle_get_low_stock(const HttpRequest& request);
    HttpResponse handle_get_daily_stats(const HttpRequest& request);
    
    // 辅助函数：从路径中提取参数
    int extract_id_from_path(const std::string& path);
    
    // 辅助函数：JSON 响应
    HttpResponse json_response(int status_code, const std::string& json_body);
    
    // 辅助函数：错误响应
    HttpResponse error_response(int status_code, const std::string& message);
};

#endif // SERVER_H