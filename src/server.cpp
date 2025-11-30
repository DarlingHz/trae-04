#include "server.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <regex>

// JSON 解析和生成的简单实现（为了避免依赖外部库）
namespace SimpleJSON {
    // 简单的 JSON 转义
    std::string escape(const std::string& str) {
        std::string escaped;
        for (char c : str) {
            switch (c) {
                case '"': escaped += "\\\""; break;
                case '\\': escaped += "\\\\"; break;
                case '\b': escaped += "\\b"; break;
                case '\f': escaped += "\\f"; break;
                case '\n': escaped += "\\n"; break;
                case '\r': escaped += "\\r"; break;
                case '\t': escaped += "\\t"; break;
                default: escaped += c; break;
            }
        }
        return escaped;
    }
    
    // 将 Product 转换为 JSON
    std::string product_to_json(const Product& product) {
        std::stringstream ss;
        ss << "{";
        ss << "\"id\": " << product.id << ",";
        ss << "\"name\": \"" << escape(product.name) << "\",";
        ss << "\"sku\": \"" << escape(product.sku) << "\",";
        ss << "\"price\": " << product.price << ",";
        ss << "\"stock\": " << product.stock << ",";
        ss << "\"reorder_threshold\": " << product.reorder_threshold << ",";
        ss << "\"created_at\": \"" << escape(product.created_at) << "\",";
        ss << "\"updated_at\": \"" << escape(product.updated_at) << "\"";
        ss << "}";
        return ss.str();
    }
    
    // 将 Product 列表转换为 JSON
    std::string products_to_json(const std::vector<Product>& products) {
        std::stringstream ss;
        ss << "[";
        for (size_t i = 0; i < products.size(); ++i) {
            if (i > 0) {
                ss << ",";
            }
            ss << product_to_json(products[i]);
        }
        ss << "]";
        return ss.str();
    }
    
    // 将 OrderItem 转换为 JSON
    std::string order_item_to_json(const OrderItem& item) {
        std::stringstream ss;
        ss << "{";
        ss << "\"id\": " << item.id << ",";
        ss << "\"order_id\": " << item.order_id << ",";
        ss << "\"product_id\": " << item.product_id << ",";
        ss << "\"quantity\": " << item.quantity << ",";
        ss << "\"unit_price\": " << item.unit_price << ",";
        ss << "\"subtotal\": " << item.subtotal << "";
        ss << "}";
        return ss.str();
    }
    
    // 将 Order 转换为 JSON
    std::string order_to_json(const Order& order, const OrderService& order_service) {
        std::stringstream ss;
        ss << "{";
        ss << "\"id\": " << order.id << ",";
        ss << "\"status\": \"" << order_service.order_status_to_string(order.status) << "\",";
        ss << "\"total_amount\": " << order.total_amount << ",";
        ss << "\"created_at\": \"" << escape(order.created_at) << "\",";
        ss << "\"updated_at\": \"" << escape(order.updated_at) << "\",";
        ss << "\"items\": [";
        for (size_t i = 0; i < order.items.size(); ++i) {
            if (i > 0) {
                ss << ",";
            }
            ss << order_item_to_json(order.items[i]);
        }
        ss << "]";
        ss << "}";
        return ss.str();
    }
    
    // 将 Order 列表转换为 JSON
    std::string orders_to_json(const std::vector<Order>& orders, const OrderService& order_service) {
        std::stringstream ss;
        ss << "[";
        for (size_t i = 0; i < orders.size(); ++i) {
            if (i > 0) {
                ss << ",";
            }
            ss << order_to_json(orders[i], order_service);
        }
        ss << "]";
        return ss.str();
    }
    
    // 将 DailyStats 转换为 JSON
    std::string daily_stats_to_json(const DailyStats& stats) {
        std::stringstream ss;
        ss << "{";
        ss << "\"date\": \"" << escape(stats.date) << "\",";
        ss << "\"total_orders\": " << stats.total_orders << ",";
        ss << "\"paid_orders\": " << stats.paid_orders << ",";
        ss << "\"paid_amount\": " << stats.paid_amount << ",";
        ss << "\"shipped_orders\": " << stats.shipped_orders << ",";
        ss << "\"shipped_amount\": " << stats.shipped_amount << ",";
        ss << "\"cancelled_orders\": " << stats.cancelled_orders << "";
        ss << "}";
        return ss.str();
    }
    
    // 简单的 JSON 解析，提取字段值
    std::map<std::string, std::string> parse_object(const std::string& json) {
        std::map<std::string, std::string> result;
        
        // 移除空格和换行符
        std::string cleaned;
        for (char c : json) {
            if (c != ' ' && c != '\n' && c != '\r' && c != '\t') {
                cleaned += c;
            }
        }
        
        // 移除首尾的 {}
        if (cleaned.size() < 2 || cleaned[0] != '{' || cleaned.back() != '}') {
            return result;
        }
        
        cleaned = cleaned.substr(1, cleaned.size() - 2);
        
        // 分割键值对
        size_t pos = 0;
        while (pos < cleaned.size()) {
            // 找到键的开始和结束
            size_t key_start = pos;
            size_t key_end = cleaned.find('"', key_start + 1);
            if (key_end == std::string::npos) {
                break;
            }
            
            std::string key = cleaned.substr(key_start + 1, key_end - key_start - 1);
            
            // 找到值的开始
            size_t value_start = cleaned.find(':', key_end + 1);
            if (value_start == std::string::npos) {
                break;
            }
            value_start++;
            
            // 确定值的类型并找到结束位置
            size_t value_end;
            if (cleaned[value_start] == '"') {
                // 字符串值
                value_end = cleaned.find('"', value_start + 1);
                if (value_end == std::string::npos) {
                    break;
                }
                
                std::string value = cleaned.substr(value_start + 1, value_end - value_start - 1);
                result[key] = value;
                
                value_end++;
            } else {
                // 数值或布尔值
                value_end = cleaned.find(',', value_start);
                if (value_end == std::string::npos) {
                    value_end = cleaned.size();
                }
                
                std::string value = cleaned.substr(value_start, value_end - value_start);
                result[key] = value;
            }
            
            // 找到下一个键值对的开始
            pos = cleaned.find('"', value_end);
            if (pos == std::string::npos) {
                break;
            }
        }
        
        return result;
    }
    
    // 解析数组
    std::vector<std::map<std::string, std::string>> parse_array(const std::string& json) {
        std::vector<std::map<std::string, std::string>> result;
        
        // 移除空格和换行符
        std::string cleaned;
        for (char c : json) {
            if (c != ' ' && c != '\n' && c != '\r' && c != '\t') {
                cleaned += c;
            }
        }
        
        // 移除首尾的 []
        if (cleaned.size() < 2 || cleaned[0] != '[' || cleaned.back() != ']') {
            return result;
        }
        
        cleaned = cleaned.substr(1, cleaned.size() - 2);
        
        // 分割数组元素
        size_t pos = 0;
        while (pos < cleaned.size()) {
            // 找到对象的开始
            size_t obj_start = cleaned.find('{', pos);
            if (obj_start == std::string::npos) {
                break;
            }
            
            // 找到对象的结束
            int brace_count = 1;
            size_t obj_end = obj_start + 1;
            while (obj_end < cleaned.size() && brace_count > 0) {
                if (cleaned[obj_end] == '{') {
                    brace_count++;
                } else if (cleaned[obj_end] == '}') {
                    brace_count--;
                }
                obj_end++;
            }
            
            // 解析对象
            std::string obj_str = cleaned.substr(obj_start, obj_end - obj_start);
            auto obj = parse_object(obj_str);
            result.push_back(obj);
            
            // 找到下一个对象的开始
            pos = obj_end;
        }
        
        return result;
    }
}

HttpServer::HttpServer(int port, Database& db) 
    : port_(port), server_fd_(-1), is_running_(false), db_(db),
      product_service_(db), order_service_(db), stats_service_(db) {
}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::start() {
    // 创建套接字
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ == -1) {
        std::cerr << "无法创建套接字" << std::endl;
        return;
    }
    
    // 设置套接字选项，允许地址重用
    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        std::cerr << "无法设置套接字选项" << std::endl;
        close(server_fd_);
        return;
    }
    
    // 绑定地址和端口
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);
    
    if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) == -1) {
        std::cerr << "无法绑定地址和端口" << std::endl;
        close(server_fd_);
        return;
    }
    
    // 监听连接
    if (listen(server_fd_, 10) == -1) {
        std::cerr << "无法监听连接" << std::endl;
        close(server_fd_);
        return;
    }
    
    is_running_ = true;
    std::cout << "服务器已启动，监听端口 " << port_ << std::endl;
    
    // 初始化路由
    init_routes();
    
    // 接受客户端连接的线程
    std::thread accept_thread([this]() {
        while (is_running_) {
            sockaddr_in client_address;
            socklen_t client_address_len = sizeof(client_address);
            
            // 接受客户端连接
            int client_fd = accept(server_fd_, (struct sockaddr*)&client_address, &client_address_len);
            if (client_fd == -1) {
                if (is_running_) {
                    std::cerr << "无法接受客户端连接" << std::endl;
                }
                continue;
            }
            
            // 处理客户端连接的线程
            std::thread client_thread([this, client_fd]() {
                handle_client(client_fd);
            });
            client_thread.detach();
        }
    });
    accept_thread.detach();
}

void HttpServer::stop() {
    if (is_running_) {
        is_running_ = false;
        close(server_fd_);
        std::cout << "服务器已停止" << std::endl;
    }
}

void HttpServer::init_routes() {
    // 商品相关路由
    routes_["POST"]["/products"] = std::bind(&HttpServer::handle_create_product, this, std::placeholders::_1);
    routes_["GET"]["/products"] = std::bind(&HttpServer::handle_get_products, this, std::placeholders::_1);
    routes_["GET"]["/products/([0-9]+)"] = std::bind(&HttpServer::handle_get_product, this, std::placeholders::_1);
    routes_["PUT"]["/products/([0-9]+)"] = std::bind(&HttpServer::handle_update_product, this, std::placeholders::_1);
    routes_["POST"]["/products/([0-9]+)/adjust_stock"] = std::bind(&HttpServer::handle_adjust_stock, this, std::placeholders::_1);
    
    // 订单相关路由
    routes_["POST"]["/orders"] = std::bind(&HttpServer::handle_create_order, this, std::placeholders::_1);
    routes_["GET"]["/orders/([0-9]+)"] = std::bind(&HttpServer::handle_get_order, this, std::placeholders::_1);
    routes_["GET"]["/orders"] = std::bind(&HttpServer::handle_get_orders, this, std::placeholders::_1);
    routes_["POST"]["/orders/([0-9]+)/status"] = std::bind(&HttpServer::handle_update_order_status, this, std::placeholders::_1);
    
    // 统计相关路由
    routes_["GET"]["/stats/low_stock"] = std::bind(&HttpServer::handle_get_low_stock, this, std::placeholders::_1);
    routes_["GET"]["/stats/daily_summary"] = std::bind(&HttpServer::handle_get_daily_stats, this, std::placeholders::_1);
}

void HttpServer::handle_client(int client_fd) {
    char buffer[4096];
    std::string request_str;
    
    // 读取客户端请求
    while (true) {
        int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read == -1) {
            std::cerr << "读取客户端请求失败" << std::endl;
            close(client_fd);
            return;
        } else if (bytes_read == 0) {
            // 客户端关闭连接
            close(client_fd);
            return;
        }
        
        buffer[bytes_read] = '\0';
        request_str += buffer;
        
        // 检查是否读取到完整的请求
        if (request_str.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }
    
    // 解析 HTTP 请求
    HttpRequest request = parse_request(request_str);
    
    // 查找匹配的路由
    RouteHandler handler = nullptr;
    int extracted_id = -1;
    
    auto method_it = routes_.find(request.method);
    if (method_it != routes_.end()) {
        for (const auto& route : method_it->second) {
            std::regex route_regex(route.first);
            std::smatch match;
            
            if (std::regex_match(request.path, match, route_regex)) {
                handler = route.second;
                
                // 提取 ID 参数（如果有）
                if (match.size() > 1) {
                    extracted_id = std::stoi(match[1].str());
                }
                
                break;
            }
        }
    }
    
    HttpResponse response;
    
    if (handler != nullptr) {
        // 临时保存提取的 ID 到请求的查询参数中，以便处理函数使用
        if (extracted_id != -1) {
            request.query_params["id"] = std::to_string(extracted_id);
        }
        
        // 调用路由处理函数
        response = handler(request);
    } else {
        // 未找到匹配的路由
        response = error_response(404, "未找到该接口");
    }
    
    // 发送 HTTP 响应
    std::string response_str = build_response(response);
    send(client_fd, response_str.c_str(), response_str.size(), 0);
    
    // 关闭客户端连接
    close(client_fd);
}

HttpRequest HttpServer::parse_request(const std::string& request_str) {
    HttpRequest request;
    std::istringstream request_stream(request_str);
    std::string line;
    
    // 解析请求行
    if (std::getline(request_stream, line)) {
        std::istringstream line_stream(line);
        line_stream >> request.method >> request.path >> request.headers["HTTP-Version"];
        
        // 解析查询参数
        size_t query_pos = request.path.find('?');
        if (query_pos != std::string::npos) {
            std::string query_str = request.path.substr(query_pos + 1);
            request.path = request.path.substr(0, query_pos);
            
            size_t pos = 0;
            while (pos < query_str.size()) {
                size_t equal_pos = query_str.find('=', pos);
                size_t ampersand_pos = query_str.find('&', pos);
                
                std::string key, value;
                if (equal_pos != std::string::npos) {
                    key = query_str.substr(pos, equal_pos - pos);
                    
                    if (ampersand_pos != std::string::npos) {
                        value = query_str.substr(equal_pos + 1, ampersand_pos - equal_pos - 1);
                    } else {
                        value = query_str.substr(equal_pos + 1);
                    }
                } else if (ampersand_pos != std::string::npos) {
                    key = query_str.substr(pos, ampersand_pos - pos);
                } else {
                    key = query_str.substr(pos);
                }
                
                request.query_params[key] = value;
                
                if (ampersand_pos != std::string::npos) {
                    pos = ampersand_pos + 1;
                } else {
                    break;
                }
            }
        }
    }
    
    // 解析请求头
    while (std::getline(request_stream, line)) {
        // 移除换行符
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        // 检查是否到达请求头的结束
        if (line.empty()) {
            break;
        }
        
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            
            // 移除值前面的空格
            size_t value_start = value.find_first_not_of(' ');
            if (value_start != std::string::npos) {
                value = value.substr(value_start);
            }
            
            request.headers[key] = value;
        }
    }
    
    // 解析请求体
    std::string body;
    while (std::getline(request_stream, line)) {
        body += line;
        // 保留换行符
        if (!request_stream.eof()) {
            body += '\n';
        }
    }
    request.body = body;
    
    return request;
}

std::string HttpServer::build_response(const HttpResponse& response) {
    std::stringstream response_stream;
    
    // 响应行
    response_stream << "HTTP/1.1 " << response.status_code << " ";
    
    // 响应状态码对应的消息
    switch (response.status_code) {
        case 200: response_stream << "OK";
        case 400: response_stream << "Bad Request";
        case 404: response_stream << "Not Found";
        case 500: response_stream << "Internal Server Error";
        default: response_stream << "Unknown";
    }
    response_stream << "\r\n";
    
    // 响应头
    for (const auto& header : response.headers) {
        response_stream << header.first << ": " << header.second << "\r\n";
    }
    
    // 内容长度头
    response_stream << "Content-Length: " << response.body.size() << "\r\n";
    
    // 响应头结束
    response_stream << "\r\n";
    
    // 响应体
    response_stream << response.body;
    
    return response_stream.str();
}

// 商品相关路由处理函数
HttpResponse HttpServer::handle_create_product(const HttpRequest& request) {
    try {
        // 解析请求体
        auto json_obj = SimpleJSON::parse_object(request.body);
        
        // 提取商品信息
        std::string name = json_obj["name"];
        std::string sku = json_obj["sku"];
        double price = std::stod(json_obj["price"]);
        int initial_stock = std::stoi(json_obj["initial_stock"]);
        int reorder_threshold = std::stoi(json_obj["reorder_threshold"]);
        
        // 创建商品
        Product product = product_service_.create_product(name, sku, price, initial_stock, reorder_threshold);
        
        if (product.id == -1) {
            return error_response(500, "创建商品失败");
        }
        
        // 返回商品信息
        std::string json_str = SimpleJSON::product_to_json(product);
        return json_response(201, json_str);
    } catch (const std::exception& e) {
        std::cerr << "创建商品失败: " << e.what() << std::endl;
        return error_response(500, "创建商品失败");
    }
}

HttpResponse HttpServer::handle_get_products(const HttpRequest& request) {
    try {
        // 提取查询参数
        int page = 1;
        int page_size = 10;
        std::string keyword = "";
        
        auto page_it = request.query_params.find("page");
        if (page_it != request.query_params.end()) {
            page = std::stoi(page_it->second);
        }
        
        auto page_size_it = request.query_params.find("page_size");
        if (page_size_it != request.query_params.end()) {
            page_size = std::stoi(page_size_it->second);
        }
        
        auto keyword_it = request.query_params.find("keyword");
        if (keyword_it != request.query_params.end()) {
            keyword = keyword_it->second;
        }
        
        // 获取商品列表
        std::vector<Product> products = product_service_.get_products(page, page_size, keyword);
        int total_count = product_service_.get_product_count(keyword);
        int total_pages = (total_count + page_size - 1) / page_size;
        
        // 构建响应 JSON
        std::stringstream ss;
        ss << "{";
        ss << "\"products\": " << SimpleJSON::products_to_json(products) << ",";
        ss << "\"pagination\": {";
        ss << "\"page\": " << page << ",";
        ss << "\"page_size\": " << page_size << ",";
        ss << "\"total_count\": " << total_count << ",";
        ss << "\"total_pages\": " << total_pages << "";
        ss << "}";
        ss << "}";
        
        return json_response(200, ss.str());
    } catch (const std::exception& e) {
        std::cerr << "获取商品列表失败: " << e.what() << std::endl;
        return error_response(500, "获取商品列表失败");
    }
}

HttpResponse HttpServer::handle_get_product(const HttpRequest& request) {
    try {
        // 提取商品 ID
        auto id_it = request.query_params.find("id");
        if (id_it == request.query_params.end()) {
            return error_response(400, "缺少商品 ID");
        }
        int product_id = std::stoi(id_it->second);
        
        // 获取商品信息
        Product product = product_service_.get_product(product_id);
        
        if (product.id == -1) {
            return error_response(404, "商品不存在");
        }
        
        // 返回商品信息
        std::string json_str = SimpleJSON::product_to_json(product);
        return json_response(200, json_str);
    } catch (const std::exception& e) {
        std::cerr << "获取商品信息失败: " << e.what() << std::endl;
        return error_response(500, "获取商品信息失败");
    }
}

HttpResponse HttpServer::handle_update_product(const HttpRequest& request) {
    try {
        // 提取商品 ID
        auto id_it = request.query_params.find("id");
        if (id_it == request.query_params.end()) {
            return error_response(400, "缺少商品 ID");
        }
        int product_id = std::stoi(id_it->second);
        
        // 解析请求体
        auto json_obj = SimpleJSON::parse_object(request.body);
        
        // 提取商品信息
        auto name_it = json_obj.find("name");
        if (name_it == json_obj.end()) {
            return error_response(400, "缺少商品名称");
        }
        std::string name = name_it->second;
        
        auto price_it = json_obj.find("price");
        if (price_it == json_obj.end()) {
            return error_response(400, "缺少商品价格");
        }
        double price = std::stod(price_it->second);
        
        auto reorder_threshold_it = json_obj.find("reorder_threshold");
        if (reorder_threshold_it == json_obj.end()) {
            return error_response(400, "缺少商品补货阈值");
        }
        int reorder_threshold = std::stoi(reorder_threshold_it->second);
        
        // 更新商品
        Product product = product_service_.update_product(product_id, name, price, reorder_threshold);
        
        if (product.id == -1) {
            return error_response(500, "更新商品失败");
        }
        
        // 返回商品信息
        std::string json_str = SimpleJSON::product_to_json(product);
        return json_response(200, json_str);
    } catch (const std::exception& e) {
        std::cerr << "更新商品失败: " << e.what() << std::endl;
        return error_response(500, "更新商品失败");
    }
}

HttpResponse HttpServer::handle_adjust_stock(const HttpRequest& request) {
    try {
        // 提取商品 ID
        auto id_it = request.query_params.find("id");
        if (id_it == request.query_params.end()) {
            return error_response(400, "缺少商品 ID");
        }
        int product_id = std::stoi(id_it->second);
        
        // 解析请求体
        auto json_obj = SimpleJSON::parse_object(request.body);
        
        // 提取库存调整信息
        auto delta_it = json_obj.find("delta");
        if (delta_it == json_obj.end()) {
            return error_response(400, "缺少库存调整量");
        }
        int delta = std::stoi(delta_it->second);
        
        auto reason_it = json_obj.find("reason");
        if (reason_it == json_obj.end()) {
            return error_response(400, "缺少库存调整原因");
        }
        std::string reason = reason_it->second;
        
        // 调整库存
        bool success = product_service_.adjust_stock(product_id, delta, reason);
        
        if (!success) {
            return error_response(400, "库存调整失败");
        }
        
        // 返回成功响应
        return json_response(200, "{\"message\": \"库存调整成功\"}");
    } catch (const std::exception& e) {
        std::cerr << "库存调整失败: " << e.what() << std::endl;
        return error_response(500, "库存调整失败");
    }
}

// 订单相关路由处理函数
HttpResponse HttpServer::handle_create_order(const HttpRequest& request) {
    try {
        // 解析请求体
        auto json_obj = SimpleJSON::parse_object(request.body);
        
        // 解析订单商品项
        std::vector<OrderCreateItem> items;
        
        // 找到 items 数组的位置
        size_t items_start = request.body.find("\"items\":[");
        if (items_start != std::string::npos) {
            items_start += strlen("\"items\":[");
            size_t items_end = request.body.find("]", items_start);
            
            if (items_end != std::string::npos) {
                std::string items_str = "[" + request.body.substr(items_start, items_end - items_start) + "]";
                auto items_array = SimpleJSON::parse_array(items_str);
                
                for (const auto& item_obj : items_array) {
                    OrderCreateItem item;
                    auto product_id_it = item_obj.find("product_id");
                    if (product_id_it != item_obj.end()) {
                        item.product_id = std::stoi(product_id_it->second);
                    } else {
                        return error_response(400, "订单商品项缺少 product_id");
                    }
                    
                    auto quantity_it = item_obj.find("quantity");
                    if (quantity_it != item_obj.end()) {
                        item.quantity = std::stoi(quantity_it->second);
                    } else {
                        return error_response(400, "订单商品项缺少 quantity");
                    }
                    
                    items.push_back(item);
                }
            }
        }
        
        // 创建订单
        Order order = order_service_.create_order(items);
        
        if (order.id == -1) {
            return error_response(400, "创建订单失败，可能是库存不足或商品不存在");
        }
        
        // 返回订单信息
        std::string json_str = SimpleJSON::order_to_json(order, order_service_);
        return json_response(201, json_str);
    } catch (const std::exception& e) {
        std::cerr << "创建订单失败: " << e.what() << std::endl;
        return error_response(500, "创建订单失败");
    }
}

HttpResponse HttpServer::handle_get_order(const HttpRequest& request) {
    try {
        // 提取订单 ID
        auto id_it = request.query_params.find("id");
        if (id_it == request.query_params.end()) {
            return error_response(400, "缺少订单 ID");
        }
        int order_id = std::stoi(id_it->second);
        
        // 获取订单信息
        Order order = order_service_.get_order(order_id);
        
        if (order.id == -1) {
            return error_response(404, "订单不存在");
        }
        
        // 返回订单信息
        std::string json_str = SimpleJSON::order_to_json(order, order_service_);
        return json_response(200, json_str);
    } catch (const std::exception& e) {
        std::cerr << "获取订单信息失败: " << e.what() << std::endl;
        return error_response(500, "获取订单信息失败");
    }
}

HttpResponse HttpServer::handle_get_orders(const HttpRequest& request) {
    try {
        // 提取查询参数
        int page = 1;
        int page_size = 10;
        OrderStatus status = OrderStatus::PENDING;
        std::string start_date = "";
        std::string end_date = "";
        
        auto page_it = request.query_params.find("page");
        if (page_it != request.query_params.end()) {
            page = std::stoi(page_it->second);
        }
        
        auto page_size_it = request.query_params.find("page_size");
        if (page_size_it != request.query_params.end()) {
            page_size = std::stoi(page_size_it->second);
        }
        
        auto status_it = request.query_params.find("status");
        if (status_it != request.query_params.end()) {
            std::string status_str = status_it->second;
            if (status_str == "PAID") {
                status = OrderStatus::PAID;
            } else if (status_str == "CANCELLED") {
                status = OrderStatus::CANCELLED;
            } else if (status_str == "SHIPPED") {
                status = OrderStatus::SHIPPED;
            } else {
                status = OrderStatus::PENDING;
            }
        }
        
        auto start_date_it = request.query_params.find("start_date");
        if (start_date_it != request.query_params.end()) {
            start_date = start_date_it->second;
        }
        
        auto end_date_it = request.query_params.find("end_date");
        if (end_date_it != request.query_params.end()) {
            end_date = end_date_it->second;
        }
        
        // 获取订单列表
        std::vector<Order> orders = order_service_.get_orders(page, page_size, status, start_date, end_date);
        int total_count = order_service_.get_order_count(status, start_date, end_date);
        int total_pages = (total_count + page_size - 1) / page_size;
        
        // 构建响应 JSON
        std::stringstream ss;
        ss << "{";
        ss << "\"orders\": " << SimpleJSON::orders_to_json(orders, order_service_) << ",";
        ss << "\"pagination\": {";
        ss << "\"page\": " << page << ",";
        ss << "\"page_size\": " << page_size << ",";
        ss << "\"total_count\": " << total_count << ",";
        ss << "\"total_pages\": " << total_pages << "";
        ss << "}";
        ss << "}";
        
        return json_response(200, ss.str());
    } catch (const std::exception& e) {
        std::cerr << "获取订单列表失败: " << e.what() << std::endl;
        return error_response(500, "获取订单列表失败");
    }
}

HttpResponse HttpServer::handle_update_order_status(const HttpRequest& request) {
    try {
        // 提取订单 ID
        auto id_it = request.query_params.find("id");
        if (id_it == request.query_params.end()) {
            return error_response(400, "缺少订单 ID");
        }
        int order_id = std::stoi(id_it->second);
        
        // 解析请求体
        auto json_obj = SimpleJSON::parse_object(request.body);
        
        // 提取状态更新信息
        OrderStatus new_status = OrderStatus::PENDING;
        if (json_obj.count("status")) {
            std::string status_str = json_obj["status"];
            if (status_str == "PAID") {
                new_status = OrderStatus::PAID;
            } else if (status_str == "CANCELLED") {
                new_status = OrderStatus::CANCELLED;
            } else if (status_str == "SHIPPED") {
                new_status = OrderStatus::SHIPPED;
            } else {
                new_status = OrderStatus::PENDING;
            }
        }
        
        bool restock = false;
        if (json_obj.count("restock")) {
            restock = (json_obj["restock"] == "true");
        }
        
        // 更新订单状态
        bool success = order_service_.update_order_status(order_id, new_status, restock);
        
        if (!success) {
            return error_response(400, "更新订单状态失败，可能是状态流转不合理或订单不存在");
        }
        
        // 返回成功响应
        return json_response(200, "{\"message\": \"订单状态更新成功\"}");
    } catch (const std::exception& e) {
        std::cerr << "更新订单状态失败: " << e.what() << std::endl;
        return error_response(500, "更新订单状态失败");
    }
}

// 统计相关路由处理函数
HttpResponse HttpServer::handle_get_low_stock(const HttpRequest& request) {
    try {
        // 获取低库存商品列表
        std::vector<Product> products = product_service_.get_low_stock_products();
        
        // 返回商品列表
        std::string json_str = SimpleJSON::products_to_json(products);
        return json_response(200, json_str);
    } catch (const std::exception& e) {
        std::cerr << "获取低库存商品失败: " << e.what() << std::endl;
        return error_response(500, "获取低库存商品失败");
    }
}

HttpResponse HttpServer::handle_get_daily_stats(const HttpRequest& request) {
    try {
        // 提取查询参数
        std::string date;
        
        auto date_it = request.query_params.find("date");
        if (date_it != request.query_params.end()) {
            date = date_it->second;
        } else {
            // 如果没有提供日期，使用当前日期
            time_t now = time(nullptr);
            tm* tm_now = localtime(&now);
            char date_buf[11];
            strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", tm_now);
            date = date_buf;
        }
        
        // 获取每日统计数据
        DailyStats stats = stats_service_.get_daily_stats(date);
        
        // 返回统计数据
        std::string json_str = SimpleJSON::daily_stats_to_json(stats);
        return json_response(200, json_str);
    } catch (const std::exception& e) {
        std::cerr << "获取每日统计数据失败: " << e.what() << std::endl;
        return error_response(500, "获取每日统计数据失败");
    }
}

// 辅助函数
int HttpServer::extract_id_from_path(const std::string& path) {
    // 提取路径中的最后一个数字作为 ID
    std::regex id_regex("/([0-9]+)$");
    std::smatch match;
    
    if (std::regex_search(path, match, id_regex)) {
        return std::stoi(match[1].str());
    }
    
    return -1;
}

HttpResponse HttpServer::json_response(int status_code, const std::string& json_body) {
    HttpResponse response;
    response.status_code = status_code;
    response.headers["Content-Type"] = "application/json";
    response.body = json_body;
    return response;
}

HttpResponse HttpServer::error_response(int status_code, const std::string& message) {
    std::stringstream ss;
    ss << "{\"error\": \"" << SimpleJSON::escape(message) << "\"}";
    return json_response(status_code, ss.str());
}