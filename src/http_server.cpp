// HTTP服务器源文件

#include "http_server.h"
#include "utils.h"
#include <nlohmann/json.hpp>

#include <iostream>
#include <sstream>
#include <algorithm>
#include <thread>
#include <vector>

// HTTP服务器实现
HttpServer::HttpServer(boost::asio::io_context& io_context, int port, 
                       std::shared_ptr<Database> database, 
                       std::shared_ptr<AuthService> auth_service, 
                       std::shared_ptr<TaskService> task_service, 
                       std::shared_ptr<ProjectService> project_service, 
                       std::shared_ptr<StatsService> stats_service, 
                       std::shared_ptr<AuditLogService> audit_log_service) 
    : io_context_(io_context), 
      port_(port), 
      database_(database), 
      auth_service_(auth_service), 
      project_service_(project_service), 
      task_service_(task_service), 
      stats_service_(stats_service), 
      audit_log_service_(audit_log_service), 
      acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)), 
      thread_pool_size_(std::thread::hardware_concurrency() * 2) {
    
    if (!database_) {
        throw HttpServerException("Database instance is null");
    }
    
    if (!auth_service_) {
        throw HttpServerException("AuthService instance is null");
    }
    
    if (!project_service_) {
        throw HttpServerException("ProjectService instance is null");
    }
    
    if (!task_service_) {
        throw HttpServerException("TaskService instance is null");
    }
    
    if (!stats_service_) {
        throw HttpServerException("StatsService instance is null");
    }
    
    if (!audit_log_service_) {
        throw HttpServerException("AuditLogService instance is null");
    }
}

HttpServer::~HttpServer() {
    Stop();
}

void HttpServer::Start() {
    try {
        // 启动接受连接
        Accept();
        
        // 创建线程池
        std::vector<std::thread> threads;
        for (std::size_t i = 0; i < thread_pool_size_; ++i) {
            threads.emplace_back([this]() {
                io_context_.run();
            });
        }
        
        std::cout << "HTTP server started on port 8080" << std::endl;
        
        // 等待所有线程完成
        for (auto& thread : threads) {
            thread.join();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to start HTTP server: " << e.what() << std::endl;
        throw HttpServerException("Failed to start HTTP server: " + std::string(e.what()));
    }
}

void HttpServer::Stop() {
    io_context_.stop();
}

void HttpServer::Accept() {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
    
    acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code& error) {
        if (!error) {
            // 处理新连接
            HandleRequest(socket);
        } else {
            std::cerr << "Failed to accept connection: " << error.message() << std::endl;
        }
        
        // 继续接受下一个连接
        Accept();
    });
}

void HttpServer::HandleRequest(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    auto buffer = std::make_shared<boost::asio::streambuf>();
    
    boost::asio::async_read_until(*socket, *buffer, "\r\n\r\n", [this, socket, buffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
        if (!error) {
            try {
                // 解析HTTP请求
                std::optional<HttpRequest> request_opt = ParseHttpRequest(buffer, socket);
                if (!request_opt) {
                    // 处理请求解析错误
                    HttpResponse error_response;
                    error_response.status_code = 400;
                    error_response.status_message = "Bad Request";
                    error_response.headers["Content-Type"] = "application/json";
                    
                    nlohmann::json error_body;
                    error_body["code"] = 400;
                    error_body["message"] = "Bad Request";
                    error_body["data"] = nullptr;
                    
                    error_response.body = error_body.dump();
                    
                    SendHttpResponse(socket, error_response);
                    return;
                }
                
                // 处理HTTP请求
                HttpResponse response = HandleRequest(request_opt.value());
                
                // 发送HTTP响应
                SendHttpResponse(socket, response);
                
            } catch (const std::exception& e) {
                std::cerr << "Failed to handle request: " << e.what() << std::endl;
                
                // 发送错误响应
                HttpResponse error_response;
                error_response.status_code = 500;
                error_response.status_message = "Internal Server Error";
                error_response.headers["Content-Type"] = "application/json";
                
                nlohmann::json error_body;
                error_body["code"] = 500;
                error_body["message"] = "Internal Server Error";
                error_body["data"] = nullptr;
                
                error_response.body = error_body.dump();
                
                SendHttpResponse(socket, error_response);
            }
        } else {
            std::cerr << "Failed to read request: " << error.message() << std::endl;
        }
    });
}

std::optional<HttpRequest> HttpServer::ParseHttpRequest(std::shared_ptr<boost::asio::streambuf> buffer, std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    HttpRequest request;
    std::istream input(buffer.get());
    
    // 解析请求行
    std::string request_line;
    std::getline(input, request_line);
    request_line.erase(request_line.find_last_not_of("\r\n") + 1);
    
    std::istringstream request_line_stream(request_line);
    request_line_stream >> request.method >> request.path >> request.version;
    
    // 解析请求头
    std::string header_line;
    while (std::getline(input, header_line) && header_line != "\r") {
        header_line.erase(header_line.find_last_not_of("\r") + 1);
        
        size_t colon_pos = header_line.find(':');
        if (colon_pos != std::string::npos) {
            std::string header_name = header_line.substr(0, colon_pos);
            std::string header_value = header_line.substr(colon_pos + 1);
            
            // 修剪头部值
            header_value = string_utils::Trim(header_value);
            
            request.headers[header_name] = header_value;
        }
    }
    
    // 解析请求体（如果有）
    if (request.headers.count("Content-Length")) {
        size_t content_length = std::stoul(request.headers["Content-Length"]);
        
        if (content_length > 0) {
            // 检查缓冲区中是否已经有足够的数据
            size_t available = buffer->size() - buffer->in_avail();
            if (available < content_length) {
                // 读取剩余的数据
            boost::system::error_code ec;
            boost::asio::read(*socket, *buffer, boost::asio::transfer_exactly(content_length - available), ec);
            if (ec) {
                // 处理读取错误
                return std::nullopt;
            }
            }
            
            // 读取请求体
            std::vector<char> body_data(content_length);
            input.read(body_data.data(), content_length);
            if (!input) {
                // 处理读取错误
                return std::nullopt;
            }
            
            request.body = std::string(body_data.begin(), body_data.end());
        }
    }
    
    // 解析查询参数
    size_t query_pos = request.path.find('?');
    if (query_pos != std::string::npos) {
        std::string query_string = request.path.substr(query_pos + 1);
        request.path = request.path.substr(0, query_pos);
        
        // 分割查询参数
        std::vector<std::string> query_params = string_utils::Split(query_string, '&');
        for (const auto& param : query_params) {
            size_t equal_pos = param.find('=');
            if (equal_pos != std::string::npos) {
                std::string param_name = url_utils::UrlDecode(param.substr(0, equal_pos));
                std::string param_value = url_utils::UrlDecode(param.substr(equal_pos + 1));
                
                request.query_params[param_name] = param_value;
            } else {
                std::string param_name = url_utils::UrlDecode(param);
                request.query_params[param_name] = "";
            }
        }
    }
    
    return request;
}

HttpResponse HttpServer::HandleRequest(const HttpRequest& request) {
    // 路由请求
    if (request.method == "POST" && request.path == "/api/v1/users/register") {
        return HandleUserRegister(request);
    } else if (request.method == "POST" && request.path == "/api/v1/users/login") {
        return HandleUserLogin(request);
    } else if (request.method == "POST" && request.path == "/api/v1/projects") {
        return HandleCreateProject(request);
    } else if (request.method == "GET" && request.path == "/api/v1/projects") {
        return HandleGetProjects(request);
    } else if (request.method == "GET" && request.path.find("/api/v1/projects/") == 0) {
        // 提取项目ID
        std::string project_id_str = request.path.substr(std::string("/api/v1/projects/").length());
        int project_id = std::stoi(project_id_str);
        
        return HandleGetProject(request, project_id);
    } else if (request.method == "POST" && request.path.find("/api/v1/projects/") == 0 && request.path.find("/tasks") != std::string::npos) {
        // 提取项目ID
        size_t tasks_pos = request.path.find("/tasks");
        std::string project_id_str = request.path.substr(std::string("/api/v1/projects/").length(), tasks_pos - std::string("/api/v1/projects/").length());
        int project_id = std::stoi(project_id_str);
        
        return HandleCreateTask(request, project_id);
    } else if (request.method == "GET" && request.path.find("/api/v1/projects/") == 0 && request.path.find("/tasks") != std::string::npos) {
        // 提取项目ID
        size_t tasks_pos = request.path.find("/tasks");
        std::string project_id_str = request.path.substr(std::string("/api/v1/projects/").length(), tasks_pos - std::string("/api/v1/projects/").length());
        int project_id = std::stoi(project_id_str);
        
        return HandleGetTasksByProject(request, project_id);
    } else if (request.method == "GET" && request.path == "/api/v1/tasks/search") {
        return HandleSearchTasks(request);
    } else if (request.method == "GET" && request.path.find("/api/v1/tasks/") == 0) {
        // 提取任务ID
        std::string task_id_str = request.path.substr(std::string("/api/v1/tasks/").length());
        int task_id = std::stoi(task_id_str);
        
        return HandleGetTask(request, task_id);
    } else if (request.method == "PATCH" && request.path.find("/api/v1/tasks/") == 0) {
        // 提取任务ID
        std::string task_id_str = request.path.substr(std::string("/api/v1/tasks/").length());
        int task_id = std::stoi(task_id_str);
        
        return HandleUpdateTask(request, task_id);
    } else if (request.method == "DELETE" && request.path.find("/api/v1/tasks/") == 0) {
        // 提取任务ID
        std::string task_id_str = request.path.substr(std::string("/api/v1/tasks/").length());
        int task_id = std::stoi(task_id_str);
        
        return HandleDeleteTask(request, task_id);
    } else if (request.method == "GET" && request.path == "/api/v1/stats/overview") {
        return HandleGetStatsOverview(request);
    } else if (request.method == "GET" && request.path == "/api/v1/audit_logs") {
        return HandleGetAuditLogs(request);
    } else {
        // 未找到路由
        HttpResponse response;
        response.status_code = 404;
        response.status_message = "Not Found";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 404;
        error_body["message"] = "Route not found";
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
    }
}

void HttpServer::SendHttpResponse(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const HttpResponse& response) {
    std::ostringstream response_stream;
    
    // 写入响应行
    response_stream << "HTTP/1.1 " << response.status_code << " " << response.status_message << "\r\n";
    
    // 写入响应头
    for (const auto& header : response.headers) {
        response_stream << header.first << ": " << header.second << "\r\n";
    }
    
    // 写入Content-Length头
    response_stream << "Content-Length: " << response.body.size() << "\r\n";
    
    // 写入空行分隔头和体
    response_stream << "\r\n";
    
    // 写入响应体
    response_stream << response.body;
    
    // 发送响应
    boost::asio::async_write(*socket, boost::asio::buffer(response_stream.str()), [socket](const boost::system::error_code& error, std::size_t bytes_transferred) {
        if (error) {
            std::cerr << "Failed to send response: " << error.message() << std::endl;
        }
        
        // 关闭连接
        socket->close();
    });
}

std::optional<User> HttpServer::AuthenticateUser(const HttpRequest& request) {
    // 检查请求头中是否包含Authorization头
    if (request.headers.count("Authorization") == 0) {
        return std::nullopt;
    }
    
    // 获取Authorization头部值
    auto it = request.headers.find("Authorization");
    std::string authorization_header = it->second;
    
    // 检查Authorization头是否以"Bearer "开头
    if (authorization_header.substr(0, 7) != "Bearer ") {
        return std::nullopt;
    }
    
    // 提取访问令牌
    std::string access_token = authorization_header.substr(7);
    
    // 验证令牌
    return auth_service_->VerifyAccessToken(access_token);
}

HttpResponse HttpServer::HandleUserRegister(const HttpRequest& request) {
    try {
        // 解析请求体
        nlohmann::json request_body = nlohmann::json::parse(request.body);
        
        // 提取注册信息
        std::string name = request_body["name"];
        std::string email = request_body["email"];
        std::string password = request_body["password"];
        
        // 注册用户
        std::optional<User> user_opt = auth_service_->Register(name, email, password);
        if (!user_opt) {
            HttpResponse response;
            response.status_code = 400;
            response.status_message = "Bad Request";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 1;
            error_body["message"] = "User registration failed";
            
            response.body = error_body.dump();
            return response;
        }
        
        User user = user_opt.value();
        
        // 生成访问令牌
        std::string access_token = auth_service_->GenerateAccessToken(user);
        
        // 记录用户注册日志
        audit_log_service_->LogUserRegister(user.id, email);
        
        // 构建响应
        HttpResponse response;
        response.status_code = 200;
        response.status_message = "OK";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json response_body;
        response_body["code"] = 0;
        response_body["message"] = "User registered successfully";
        
        nlohmann::json user_data;
        user_data["id"] = user.id;
        user_data["name"] = user.name;
        user_data["email"] = user.email;
        user_data["created_at"] = time_utils::ToIsoString(user.created_at);
        
        nlohmann::json token_data;
        token_data["access_token"] = access_token;
        token_data["expires_at"] = "";
        
        
        nlohmann::json data;
        data["user"] = user_data;
        data["token"] = token_data;
        
        response_body["data"] = data;
        
        response.body = response_body.dump();
        
        return response;
        
    } catch (const AuthException& e) {
        // 处理认证异常
        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 400;
        error_body["message"] = e.what();
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
        
    } catch (const nlohmann::json::parse_error& e) {
        // 处理JSON解析错误
        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 400;
        error_body["message"] = "Invalid JSON format";
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
        
    } catch (const std::exception& e) {
        // 处理其他异常
        HttpResponse response;
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 500;
        error_body["message"] = "Internal Server Error";
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
    }
}

HttpResponse HttpServer::HandleUserLogin(const HttpRequest& request) {
    try {
        // 解析请求体
        nlohmann::json request_body = nlohmann::json::parse(request.body);
        
        // 提取登录信息
        std::string email = request_body["email"];
        std::string password = request_body["password"];
        
        // 登录用户
        std::optional<User> user_opt = auth_service_->Login(email, password);
        if (!user_opt) {
            HttpResponse response;
            response.status_code = 401;
            response.status_message = "Unauthorized";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 1;
            error_body["message"] = "Invalid email or password";
            
            response.body = error_body.dump();
            return response;
        }
        
        User user = user_opt.value();
        
        // 生成访问令牌
        std::string access_token = auth_service_->GenerateAccessToken(user);
        
        // 记录用户登录日志
        audit_log_service_->LogUserLogin(user.id, email);
        
        // 构建响应
        HttpResponse response;
        response.status_code = 200;
        response.status_message = "OK";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json response_body;
        response_body["code"] = 0;
        response_body["message"] = "User logged in successfully";
        
        nlohmann::json user_data;
        user_data["id"] = user.id;
        user_data["name"] = user.name;
        user_data["email"] = user.email;
        user_data["created_at"] = time_utils::ToIsoString(user.created_at);
        
        nlohmann::json token_data;
        token_data["access_token"] = access_token;
        token_data["expires_at"] = "";
        
        
        nlohmann::json data;
        data["user"] = user_data;
        data["token"] = token_data;
        
        response_body["data"] = data;
        
        response.body = response_body.dump();
        
        return response;
        
    } catch (const AuthException& e) {
        // 处理认证异常
        HttpResponse response;
        response.status_code = 401;
        response.status_message = "Unauthorized";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 401;
        error_body["message"] = e.what();
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
        
    } catch (const nlohmann::json::parse_error& e) {
        // 处理JSON解析错误
        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 400;
        error_body["message"] = "Invalid JSON format";
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
        
    } catch (const std::exception& e) {
        // 处理其他异常
        HttpResponse response;
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 500;
        error_body["message"] = "Internal Server Error";
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
    }
}

HttpResponse HttpServer::HandleCreateProject(const HttpRequest& request) {
    try {
        // 认证用户
        auto user = AuthenticateUser(request);
        if (!user) {
            HttpResponse response;
            response.status_code = 401;
            response.status_message = "Unauthorized";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 401;
            error_body["message"] = "Authentication failed";
            error_body["data"] = nullptr;
            
            response.body = error_body.dump();
            
            return response;
        }
        
        // 解析请求体
        nlohmann::json request_body = nlohmann::json::parse(request.body);
        
        // 提取项目信息
        std::string name = request_body["name"];
        std::optional<std::string> description;
        if (request_body.contains("description")) {
            description = request_body["description"];
        }
        
        // 创建项目
        std::optional<Project> project_opt = project_service_->CreateProject(user->id, name, description);
        if (!project_opt) {
            HttpResponse response;
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 1;
            error_body["message"] = "Failed to create project";
            
            response.body = error_body.dump();
            return response;
        }
        
        Project project = project_opt.value();
        
        // 构建响应
        HttpResponse response;
        response.status_code = 200;
        response.status_message = "OK";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json response_body;
        response_body["code"] = 0;
        response_body["message"] = "Project created successfully";
        
        nlohmann::json project_data_json;
        project_data_json["id"] = project.id;
        project_data_json["name"] = project.name;
        if (project.description) {
            project_data_json["description"] = project.description.value();
        }
        project_data_json["owner_user_id"] = project.owner_user_id;
        project_data_json["created_at"] = time_utils::ToIsoString(project.created_at);
        
        response_body["data"] = project_data_json;
        
        response.body = response_body.dump();
        
        return response;
        
    } catch (const ProjectServiceException& e) {
        // 处理项目服务异常
        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 400;
        error_body["message"] = e.what();
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
        
    } catch (const nlohmann::json::parse_error& e) {
        // 处理JSON解析错误
        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 400;
        error_body["message"] = "Invalid JSON format";
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
        
    } catch (const std::exception& e) {
        // 处理其他异常
        HttpResponse response;
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 500;
        error_body["message"] = "Internal Server Error";
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
    }
}

HttpResponse HttpServer::HandleGetProjects(const HttpRequest& request) {
    try {
        // 认证用户
        auto user = AuthenticateUser(request);
        if (!user) {
            HttpResponse response;
            response.status_code = 401;
            response.status_message = "Unauthorized";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 401;
            error_body["message"] = "Authentication failed";
            error_body["data"] = nullptr;
            
            response.body = error_body.dump();
            
            return response;
        }
        
        // 提取分页参数
        int page = 1;
        int page_size = 10;
        
        if (request.query_params.count("page")) {
            auto page_it = request.query_params.find("page");
        if (page_it != request.query_params.end()) {
            page = std::stoi(page_it->second);
        }
        }
        
        if (request.query_params.count("page_size")) {
            auto page_size_it = request.query_params.find("page_size");
        if (page_size_it != request.query_params.end()) {
            page_size = std::stoi(page_size_it->second);
        }
        }
        
        // 验证分页参数
        if (page < 1) {
            page = 1;
        }
        
        if (page_size < 1 || page_size > 100) {
            page_size = 10;
        }
        
        // 获取用户的项目列表
        std::vector<Project> projects = project_service_->GetProjectsByUserId(user->id, page, page_size);
        
        // 获取用户的项目总数
        int total_projects = project_service_->GetProjectsCountByUserId(user->id);
        
        // 构建响应
        HttpResponse response;
        response.status_code = 200;
        response.status_message = "OK";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json response_body;
        response_body["code"] = 0;
        response_body["message"] = "OK";
        
        nlohmann::json data;
        nlohmann::json projects_json = nlohmann::json::array();
        
        for (const auto& project : projects) {
            nlohmann::json project_json;
            project_json["id"] = project.id;
            project_json["name"] = project.name;
            if (project.description) {
                project_json["description"] = project.description.value();
            }
            project_json["owner_user_id"] = project.owner_user_id;
            project_json["created_at"] = time_utils::ToIsoString(project.created_at);
            
            projects_json.push_back(project_json);
        }
        
        data["projects"] = projects_json;
        data["total"] = total_projects;
        data["page"] = page;
        data["page_size"] = page_size;
        
        response_body["data"] = data;
        
        response.body = response_body.dump();
        
        return response;
        
    } catch (const ProjectServiceException& e) {
        // 处理项目服务异常
        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 400;
        error_body["message"] = e.what();
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
        
    } catch (const std::exception& e) {
        // 处理其他异常
        HttpResponse response;
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 500;
        error_body["message"] = "Internal Server Error";
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
    }
}

HttpResponse HttpServer::HandleGetProject(const HttpRequest& request, int project_id) {
    try {
        // 认证用户
        auto user = AuthenticateUser(request);
        if (!user) {
            HttpResponse response;
            response.status_code = 401;
            response.status_message = "Unauthorized";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 401;
            error_body["message"] = "Authentication failed";
            error_body["data"] = nullptr;
            
            response.body = error_body.dump();
            
            return response;
        }
        
        // 获取项目详情
        auto project = project_service_->GetProjectById(user->id, project_id);
        if (!project) {
            HttpResponse response;
            response.status_code = 404;
            response.status_message = "Not Found";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 404;
            error_body["message"] = "Project not found";
            error_body["data"] = nullptr;
            
            response.body = error_body.dump();
            
            return response;
        }
        
        // 获取项目统计信息
        std::optional<ProjectStats> stats_opt = project_service_->GetProjectStats(user->id, project_id);
        if (!stats_opt) {
            HttpResponse response;
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 1;
            error_body["message"] = "Failed to get project stats";
            
            response.body = error_body.dump();
            return response;
        }
        
        ProjectStats stats = stats_opt.value();
        
        // 构建响应
        HttpResponse response;
        response.status_code = 200;
        response.status_message = "OK";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json response_body;
        response_body["code"] = 0;
        response_body["message"] = "OK";
        
        nlohmann::json project_json;
        project_json["id"] = project->id;
        project_json["name"] = project->name;
        if (project->description) {
            project_json["description"] = project->description.value();
        }
        project_json["owner_user_id"] = project->owner_user_id;
        project_json["created_at"] = time_utils::ToIsoString(project->created_at);
        
        nlohmann::json stats_json;
        stats_json["total_tasks"] = stats.total_tasks;
        stats_json["todo_tasks"] = stats.todo_tasks;
        stats_json["doing_tasks"] = stats.doing_tasks;
        stats_json["done_tasks"] = stats.done_tasks;
        
        nlohmann::json data;
        data["project"] = project_json;
        data["stats"] = stats_json;
        
        response_body["data"] = data;
        
        response.body = response_body.dump();
        
        return response;
        
    } catch (const ProjectServiceException& e) {
        // 处理项目服务异常
        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 400;
        error_body["message"] = e.what();
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
        
    } catch (const std::exception& e) {
        // 处理其他异常
        HttpResponse response;
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 500;
        error_body["message"] = "Internal Server Error";
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
    }
}

HttpResponse HttpServer::HandleCreateTask(const HttpRequest& request, int project_id) {
    try {
        // 认证用户
        auto user = AuthenticateUser(request);
        if (!user) {
            HttpResponse response;
            response.status_code = 401;
            response.status_message = "Unauthorized";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 401;
            error_body["message"] = "Authentication failed";
            error_body["data"] = nullptr;
            
            response.body = error_body.dump();
            
            return response;
        }
        
        // 解析请求体
        nlohmann::json request_body = nlohmann::json::parse(request.body);
        
        // 提取任务信息
        std::string title = request_body["title"];
        std::optional<std::string> description;
        if (request_body.contains("description")) {
            description = request_body["description"];
        }
        
        std::string status = "todo";  // 默认状态为待办
        if (request_body.contains("status")) {
            status = request_body["status"];
        }
        
        std::string priority = "medium";  // 默认优先级为中等
        if (request_body.contains("priority")) {
            priority = request_body["priority"];
        }
        
        std::optional<int> assignee_user_id;
        if (request_body.contains("assignee_user_id")) {
            assignee_user_id = request_body["assignee_user_id"];
        }
        
        std::optional<std::chrono::system_clock::time_point> due_date;
        if (request_body.contains("due_date")) {
            std::string due_date_str = request_body["due_date"];
            due_date = time_utils::FromIsoString(due_date_str);
        }
        
        std::optional<std::vector<std::string>> tags;
        if (request_body.contains("tags")) {
            tags = request_body["tags"].get<std::vector<std::string>>();
        }
        
        // 创建任务
        std::optional<Task> task_opt = task_service_->CreateTask(user->id, project_id, title, description, assignee_user_id, status, priority, due_date, tags.value_or(std::vector<std::string>{}));
        if (!task_opt) {
            HttpResponse response;
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 1;
            error_body["message"] = "Failed to create task";
            
            response.body = error_body.dump();
            return response;
        }
        
        Task task = task_opt.value();
        
        // 构建响应
        HttpResponse response;
        response.status_code = 200;
        response.status_message = "OK";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json response_body;
        response_body["code"] = 0;
        response_body["message"] = "Task created successfully";
        
        nlohmann::json task_json;
        task_json["id"] = task.id;
        task_json["project_id"] = task.project_id;
        task_json["title"] = task.title;
        if (task.description) {
            task_json["description"] = task.description.value();
        }
        task_json["status"] = task.status;
        task_json["priority"] = task.priority;
        if (task.assignee_user_id) {
            task_json["assignee_user_id"] = task.assignee_user_id.value();
        }
        if (task.due_date) {
            task_json["due_date"] = time_utils::ToIsoString(task.due_date.value());
        }
        task_json["created_at"] = time_utils::ToIsoString(task.created_at);
        task_json["updated_at"] = time_utils::ToIsoString(task.updated_at);
        

        
        response_body["data"] = task_json;
        
        response.body = response_body.dump();
        
        return response;
        
    } catch (const TaskServiceException& e) {
        // 处理任务服务异常
        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 400;
        error_body["message"] = e.what();
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
        
    } catch (const nlohmann::json::parse_error& e) {
        // 处理JSON解析错误
        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 400;
        error_body["message"] = "Invalid JSON format";
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
        
    } catch (const std::exception& e) {
        // 处理其他异常
        HttpResponse response;
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 500;
        error_body["message"] = "Internal Server Error";
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
    }
}

HttpResponse HttpServer::HandleGetTasksByProject(const HttpRequest& request, int project_id) {
    try {
        // 认证用户
        auto user = AuthenticateUser(request);
        if (!user) {
            HttpResponse response;
            response.status_code = 401;
            response.status_message = "Unauthorized";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 401;
            error_body["message"] = "Authentication failed";
            error_body["data"] = nullptr;
            
            response.body = error_body.dump();
            
            return response;
        }
        
        // 提取分页参数
        int page = 1;
        int page_size = 10;
        
        if (request.query_params.count("page")) {
            auto page_it = request.query_params.find("page");
            if (page_it != request.query_params.end()) {
                page = std::stoi(page_it->second);
            }
        }
        
        if (request.query_params.count("page_size")) {
            auto page_size_it = request.query_params.find("page_size");
            if (page_size_it != request.query_params.end()) {
                page_size = std::stoi(page_size_it->second);
            }
        }
        
        // 验证分页参数
        if (page < 1) {
            page = 1;
        }
        
        if (page_size < 1 || page_size > 100) {
            page_size = 10;
        }
        
        // 获取项目中的任务列表
        std::vector<Task> tasks = task_service_->GetTasksByProjectId(user->id, project_id, page, page_size);
        
        // 获取项目中的任务总数
        int total_tasks = database_->GetTasksCountByProjectId(project_id);
        
        // 构建响应
        HttpResponse response;
        response.status_code = 200;
        response.status_message = "OK";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json response_body;
        response_body["code"] = 0;
        response_body["message"] = "OK";
        
        nlohmann::json data;
        nlohmann::json tasks_json = nlohmann::json::array();
        
        for (const auto& task : tasks) {
            nlohmann::json task_json;
            task_json["id"] = task.id;
            task_json["project_id"] = task.project_id;
            task_json["title"] = task.title;
            if (task.description) {
                task_json["description"] = task.description.value();
            }
            task_json["status"] = task.status;
            task_json["priority"] = task.priority;
            if (task.assignee_user_id) {
                task_json["assignee_user_id"] = task.assignee_user_id.value();
            }
            if (task.due_date) {
                task_json["due_date"] = time_utils::ToIsoString(task.due_date.value());
            }
            task_json["created_at"] = time_utils::ToIsoString(task.created_at);
            task_json["updated_at"] = time_utils::ToIsoString(task.updated_at);
            
            tasks_json.push_back(task_json);
        }
        
        data["tasks"] = tasks_json;
        data["total"] = total_tasks;
        data["page"] = page;
        data["page_size"] = page_size;
        
        response_body["data"] = data;
        
        response.body = response_body.dump();
        
        return response;
        
    } catch (const TaskServiceException& e) {
        // 处理任务服务异常
        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 400;
        error_body["message"] = e.what();
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
        
    } catch (const std::exception& e) {
        // 处理其他异常
        HttpResponse response;
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 500;
        error_body["message"] = "Internal Server Error";
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
    }
}

HttpResponse HttpServer::HandleSearchTasks(const HttpRequest& request) {
    try {
        // 认证用户
        auto user = AuthenticateUser(request);
        if (!user) {
            HttpResponse response;
            response.status_code = 401;
            response.status_message = "Unauthorized";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 401;
            error_body["message"] = "Authentication failed";
            error_body["data"] = nullptr;
            
            response.body = error_body.dump();
            
            return response;
        }
        
        // 提取查询参数
        std::optional<std::string> keyword = std::nullopt;
        std::optional<std::string> status = std::nullopt;
        std::optional<std::string> tag = std::nullopt;
        std::optional<std::chrono::system_clock::time_point> due_before = std::nullopt;
        std::optional<std::chrono::system_clock::time_point> due_after = std::nullopt;
        int page = 1;
        int page_size = 10;
        
        // 分页参数
        if (request.query_params.count("page")) {
            auto params_page_it = request.query_params.find("page");
            if (params_page_it != request.query_params.end()) {
                page = std::stoi(params_page_it->second);
            }
        }
        
        if (request.query_params.count("page_size")) {
            auto params_page_size_it = request.query_params.find("page_size");
            if (params_page_size_it != request.query_params.end()) {
                page_size = std::stoi(params_page_size_it->second);
            }
        }
        
        // 过滤参数
        if (request.query_params.count("status")) {
            auto params_status_it = request.query_params.find("status");
            if (params_status_it != request.query_params.end()) {
                status = params_status_it->second;
            }
        }
        
        if (request.query_params.count("keyword")) {
            auto params_keyword_it = request.query_params.find("keyword");
            if (params_keyword_it != request.query_params.end()) {
                keyword = params_keyword_it->second;
            }
        }
        
        if (request.query_params.count("tag")) {
            auto params_tag_it = request.query_params.find("tag");
            if (params_tag_it != request.query_params.end()) {
                tag = params_tag_it->second;
            }
        }
        
        if (request.query_params.count("due_before")) {
            auto params_due_before_it = request.query_params.find("due_before");
            if (params_due_before_it != request.query_params.end()) {
                std::string due_before_str = params_due_before_it->second;
                due_before = time_utils::FromIsoString(due_before_str);
            }
        }
        
        if (request.query_params.count("due_after")) {
            auto params_due_after_it = request.query_params.find("due_after");
            if (params_due_after_it != request.query_params.end()) {
                std::string due_after_str = params_due_after_it->second;
                due_after = time_utils::FromIsoString(due_after_str);
            }
        }
        
        // 验证分页参数
        if (page < 1) {
            page = 1;
        }
        
        if (page_size < 1 || page_size > 100) {
            page_size = 10;
        }
        
        // 构建任务查询参数
        TaskQueryParams params;
        params.keyword = keyword;
        params.status = status;
        params.tag = tag;
        params.due_before = due_before;
        params.due_after = due_after;
        params.page = page;
        params.page_size = page_size;
        
        // 搜索任务
        std::vector<Task> tasks = task_service_->SearchTasks(user->id, params);
        
        // 获取搜索结果总数
        int total_tasks = task_service_->GetSearchTasksCount(user->id, params);
        
        // 构建响应
        HttpResponse response;
        response.status_code = 200;
        response.status_message = "OK";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json response_body;
        response_body["code"] = 0;
        response_body["message"] = "OK";
        
        nlohmann::json data;
        nlohmann::json tasks_json = nlohmann::json::array();
        
        for (const auto& task : tasks) {
            nlohmann::json task_json;
            task_json["id"] = task.id;
            task_json["project_id"] = task.project_id;
            task_json["title"] = task.title;
            if (task.description) {
                task_json["description"] = task.description.value();
            }
            task_json["status"] = task.status;
            task_json["priority"] = task.priority;
            if (task.assignee_user_id) {
                task_json["assignee_user_id"] = task.assignee_user_id.value();
            }
            if (task.due_date) {
                task_json["due_date"] = time_utils::ToIsoString(task.due_date.value());
            }
            task_json["created_at"] = time_utils::ToIsoString(task.created_at);
            task_json["updated_at"] = time_utils::ToIsoString(task.updated_at);
            
            tasks_json.push_back(task_json);
        }
        
        data["tasks"] = tasks_json;
        data["total"] = total_tasks;
        data["page"] = page;
        data["page_size"] = page_size;
        
        response_body["data"] = data;
        
        response.body = response_body.dump();
        
        return response;
        
    } catch (const TaskServiceException& e) {
        // 处理任务服务异常
        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 400;
        error_body["message"] = e.what();
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
        
    } catch (const std::exception& e) {
        // 处理其他异常
        HttpResponse response;
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 500;
        error_body["message"] = "Internal Server Error";
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
    }
}

HttpResponse HttpServer::HandleGetTask(const HttpRequest& request, int task_id) {
    try {
        // 认证用户
        auto user = AuthenticateUser(request);
        if (!user) {
            HttpResponse response;
            response.status_code = 401;
            response.status_message = "Unauthorized";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 401;
            error_body["message"] = "Authentication failed";
            error_body["data"] = nullptr;
            
            response.body = error_body.dump();
            
            return response;
        }
        
        // 获取任务详情
        auto task = task_service_->GetTaskById(user->id, task_id);
        if (!task) {
            HttpResponse response;
            response.status_code = 404;
            response.status_message = "Not Found";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 404;
            error_body["message"] = "Task not found";
            error_body["data"] = nullptr;
            
            response.body = error_body.dump();
            
            return response;
        }
        
        // 构建响应
        HttpResponse response;
        response.status_code = 200;
        response.status_message = "OK";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json response_body;
        response_body["code"] = 0;
        response_body["message"] = "OK";
        
        nlohmann::json task_json;
        task_json["id"] = task->id;
        task_json["project_id"] = task->project_id;
        task_json["title"] = task->title;
        if (task->description) {
            task_json["description"] = task->description.value();
        }
        task_json["status"] = task->status;
        task_json["priority"] = task->priority;
        if (task->assignee_user_id) {
            task_json["assignee_user_id"] = task->assignee_user_id.value();
        }
        if (task->due_date) {
            task_json["due_date"] = time_utils::ToIsoString(task->due_date.value());
        }
        task_json["created_at"] = time_utils::ToIsoString(task->created_at);
        task_json["updated_at"] = time_utils::ToIsoString(task->updated_at);
        
        response_body["data"] = task_json;
        
        response.body = response_body.dump();
        
        return response;
        
    } catch (const TaskServiceException& e) {
        // 处理任务服务异常
        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 400;
        error_body["message"] = e.what();
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
        
    } catch (const std::exception& e) {
        // 处理其他异常
        HttpResponse response;
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 500;
        error_body["message"] = "Internal Server Error";
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
    }
}

HttpResponse HttpServer::HandleUpdateTask(const HttpRequest& request, int task_id) {
    try {
        // 认证用户
        auto user = AuthenticateUser(request);
        if (!user) {
            HttpResponse response;
            response.status_code = 401;
            response.status_message = "Unauthorized";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 401;
            error_body["message"] = "Authentication failed";
            error_body["data"] = nullptr;
            
            response.body = error_body.dump();
            
            return response;
        }
        
        // 解析请求体
        nlohmann::json request_body = nlohmann::json::parse(request.body);
        
        // 提取任务更新信息
        std::optional<std::string> title = std::nullopt;
        std::optional<std::string> description = std::nullopt;
        std::optional<int> assignee_user_id = std::nullopt;
        std::optional<std::string> status = std::nullopt;
        std::optional<std::string> priority = std::nullopt;
        std::optional<std::chrono::system_clock::time_point> due_date = std::nullopt;
        
        if (request_body.contains("title")) {
            title = request_body["title"];
        }
        
        if (request_body.contains("description")) {
            description = request_body["description"];
        }
        
        if (request_body.contains("status")) {
            status = request_body["status"];
        }
        
        if (request_body.contains("priority")) {
            priority = request_body["priority"];
        }
        
        if (request_body.contains("assignee_user_id")) {
            assignee_user_id = request_body["assignee_user_id"];
        }
        
        if (request_body.contains("due_date")) {
            std::string due_date_str = request_body["due_date"];
            due_date = time_utils::FromIsoString(due_date_str);
        }
        
        // 更新任务
        std::optional<Task> updated_task_opt = task_service_->UpdateTask(user->id, task_id, title, description, assignee_user_id, status, priority, due_date);
        if (!updated_task_opt) {
            HttpResponse response;
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 500;
            error_body["message"] = "Failed to update task";
            error_body["data"] = nullptr;
            
            response.body = error_body.dump();
            
            return response;
        }
        
        Task updated_task = updated_task_opt.value();
        
        // 构建响应
        HttpResponse response;
        response.status_code = 200;
        response.status_message = "OK";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json response_body;
        response_body["code"] = 0;
        response_body["message"] = "Task updated successfully";
        
        nlohmann::json task_json;
        task_json["id"] = updated_task.id;
        task_json["project_id"] = updated_task.project_id;
        task_json["title"] = updated_task.title;
        if (updated_task.description) {
            task_json["description"] = updated_task.description.value();
        }
        task_json["status"] = updated_task.status;
        task_json["priority"] = updated_task.priority;
        if (updated_task.assignee_user_id) {
            task_json["assignee_user_id"] = updated_task.assignee_user_id.value();
        }
        if (updated_task.due_date) {
            task_json["due_date"] = time_utils::ToIsoString(updated_task.due_date.value());
        }
        task_json["created_at"] = time_utils::ToIsoString(updated_task.created_at);
        task_json["updated_at"] = time_utils::ToIsoString(updated_task.updated_at);
        

        
        response_body["data"] = task_json;
        
        response.body = response_body.dump();
        
        return response;
        
    } catch (const TaskServiceException& e) {
        // 处理任务服务异常
        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 400;
        error_body["message"] = e.what();
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
        
    } catch (const nlohmann::json::parse_error& e) {
        // 处理JSON解析错误
        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 400;
        error_body["message"] = "Invalid JSON format";
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
        
    } catch (const std::exception& e) {
        // 处理其他异常
        HttpResponse response;
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 500;
        error_body["message"] = "Internal Server Error";
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
    }
}

HttpResponse HttpServer::HandleDeleteTask(const HttpRequest& request, int task_id) {
    try {
        // 认证用户
        auto user = AuthenticateUser(request);
        if (!user) {
            HttpResponse response;
            response.status_code = 401;
            response.status_message = "Unauthorized";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 401;
            error_body["message"] = "Authentication failed";
            error_body["data"] = nullptr;
            
            response.body = error_body.dump();
            
            return response;
        }
        
        // 删除任务
        bool success = task_service_->DeleteTask(user->id, task_id);
        if (!success) {
            HttpResponse response;
            response.status_code = 404;
            response.status_message = "Not Found";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 404;
            error_body["message"] = "Task not found";
            error_body["data"] = nullptr;
            
            response.body = error_body.dump();
            
            return response;
        }
        
        // 构建响应
        HttpResponse response;
        response.status_code = 200;
        response.status_message = "OK";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json response_body;
        response_body["code"] = 0;
        response_body["message"] = "Task deleted successfully";
        response_body["data"] = nullptr;
        
        response.body = response_body.dump();
        
        return response;
        
    } catch (const TaskServiceException& e) {
        // 处理任务服务异常
        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 400;
        error_body["message"] = e.what();
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
        
    } catch (const std::exception& e) {
        // 处理其他异常
        HttpResponse response;
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 500;
        error_body["message"] = "Internal Server Error";
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
    }
}

HttpResponse HttpServer::HandleGetStatsOverview(const HttpRequest& request) {
    try {
        // 认证用户
        auto user = AuthenticateUser(request);
        if (!user) {
            HttpResponse response;
            response.status_code = 401;
            response.status_message = "Unauthorized";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 401;
            error_body["message"] = "Authentication failed";
            error_body["data"] = nullptr;
            
            response.body = error_body.dump();
            
            return response;
        }
        
        // 获取用户统计概览
        std::optional<UserStats> stats_opt = stats_service_->GetUserStatsOverview(user->id);
        if (!stats_opt) {
            HttpResponse response;
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 500;
            error_body["message"] = "Failed to get user stats";
            error_body["data"] = nullptr;
            
            response.body = error_body.dump();
            
            return response;
        }
        
        UserStats stats = stats_opt.value();
        
        // 构建响应
        HttpResponse response;
        response.status_code = 200;
        response.status_message = "OK";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json response_body;
        response_body["code"] = 0;
        response_body["message"] = "OK";
        
        nlohmann::json data;
        
        nlohmann::json task_stats_json;
        task_stats_json["todo"] = stats.task_stats.todo;
        task_stats_json["doing"] = stats.task_stats.doing;
        task_stats_json["done"] = stats.task_stats.done;
        
        data["task_stats"] = task_stats_json;
        data["overdue_tasks"] = stats.overdue_tasks;
        data["recent_tasks"] = stats.recent_tasks;
        data["total_projects"] = stats.total_projects;
        
        response_body["data"] = data;
        
        response.body = response_body.dump();
        
        return response;
        
    } catch (const StatsServiceException& e) {
        // 处理统计服务异常
        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 400;
        error_body["message"] = e.what();
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
        
    } catch (const std::exception& e) {
        // 处理其他异常
        HttpResponse response;
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 500;
        error_body["message"] = "Internal Server Error";
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
    }
}

HttpResponse HttpServer::HandleGetAuditLogs(const HttpRequest& request) {
    try {
        // 认证用户
        auto user = AuthenticateUser(request);
        if (!user) {
            HttpResponse response;
            response.status_code = 401;
            response.status_message = "Unauthorized";
            response.headers["Content-Type"] = "application/json";
            
            nlohmann::json error_body;
            error_body["code"] = 401;
            error_body["message"] = "Authentication failed";
            error_body["data"] = nullptr;
            
            response.body = error_body.dump();
            
            return response;
        }
        
        // 提取查询参数
        int limit = 10;
        if (request.query_params.count("limit")) {
            auto limit_it = request.query_params.find("limit");
        if (limit_it != request.query_params.end()) {
            limit = std::stoi(limit_it->second);
        }
        }
        
        // 验证查询参数
        if (limit <= 0 || limit > 100) {
            limit = 10;
        }
        
        // 获取用户的审计日志
        std::vector<AuditLog> audit_logs = audit_log_service_->GetUserAuditLogs(user->id, limit);
        
        // 构建响应
        HttpResponse response;
        response.status_code = 200;
        response.status_message = "OK";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json response_body;
        response_body["code"] = 0;
        response_body["message"] = "OK";
        
        nlohmann::json data;
        nlohmann::json audit_logs_json = nlohmann::json::array();
        
        for (const auto& audit_log : audit_logs) {
            nlohmann::json audit_log_json;
            audit_log_json["id"] = audit_log.id;
            audit_log_json["user_id"] = audit_log.user_id;
            audit_log_json["action_type"] = audit_log.action_type;
            audit_log_json["resource_type"] = audit_log.resource_type;
            if (audit_log.resource_id) {
                audit_log_json["resource_id"] = audit_log.resource_id.value();
            }
            audit_log_json["created_at"] = time_utils::ToIsoString(audit_log.created_at);
            if (audit_log.detail) {
                audit_log_json["detail"] = audit_log.detail.value();
            }
            
            audit_logs_json.push_back(audit_log_json);
        }
        
        data["audit_logs"] = audit_logs_json;
        data["limit"] = limit;
        
        response_body["data"] = data;
        
        response.body = response_body.dump();
        
        return response;
        
    } catch (const AuditLogServiceException& e) {
        // 处理审计日志服务异常
        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 400;
        error_body["message"] = e.what();
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
        
    } catch (const std::exception& e) {
        // 处理其他异常
        HttpResponse response;
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        response.headers["Content-Type"] = "application/json";
        
        nlohmann::json error_body;
        error_body["code"] = 500;
        error_body["message"] = "Internal Server Error";
        error_body["data"] = nullptr;
        
        response.body = error_body.dump();
        
        return response;
    }
}