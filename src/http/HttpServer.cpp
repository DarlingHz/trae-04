#include "http/HttpServer.hpp"
#include "utils/Logger.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <algorithm>

HttpServer::HttpServer(uint16_t port, uint32_t thread_pool_size) 
    : port_(port), thread_pool_size_(thread_pool_size), server_socket_(-1), is_running_(false) {
}

HttpServer::~HttpServer() {
    stop();
}

bool HttpServer::start() {
    // 创建服务器套接字
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ == -1) {
        LOG_ERROR("Failed to create server socket: " + std::string(strerror(errno)));
        return false;
    }

    // 设置套接字选项，允许地址重用
    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        LOG_ERROR("Failed to set socket options: " + std::string(strerror(errno)));
        close(server_socket_);
        server_socket_ = -1;
        return false;
    }

    // 绑定服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);

    if (bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        LOG_ERROR("Failed to bind server socket: " + std::string(strerror(errno)));
        close(server_socket_);
        server_socket_ = -1;
        return false;
    }

    // 监听连接
    if (listen(server_socket_, 10) == -1) {
        LOG_ERROR("Failed to listen on server socket: " + std::string(strerror(errno)));
        close(server_socket_);
        server_socket_ = -1;
        return false;
    }

    is_running_ = true;

    // 创建线程池
    for (uint32_t i = 0; i < thread_pool_size_; ++i) {
        thread_pool_.emplace_back(&HttpServer::acceptConnections, this);
    }

    LOG_INFO("HTTP server started successfully on port: " + std::to_string(port_));
    return true;
}

void HttpServer::stop() {
    if (!is_running_) {
        return;
    }

    is_running_ = false;

    // 关闭服务器套接字
    if (server_socket_ != -1) {
        close(server_socket_);
        server_socket_ = -1;
    }

    // 等待所有线程结束
    for (auto& thread : thread_pool_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    thread_pool_.clear();

    LOG_INFO("HTTP server stopped successfully");
}

void HttpServer::registerHandler(const std::string& method, const std::string& path, RequestHandler handler) {
    HandlerKey key(method, path);
    handlers_[key] = handler;
}

void HttpServer::acceptConnections() {
    while (is_running_) {
        // 接受客户端连接
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket_, (struct sockaddr*)&client_addr, &client_addr_len);

        if (client_socket == -1) {
            if (is_running_) {
                LOG_ERROR("Failed to accept client connection: " + std::string(strerror(errno)));
            }
            continue;
        }

        LOG_INFO("New client connection from: " + std::string(inet_ntoa(client_addr.sin_addr)) + ":" + std::to_string(ntohs(client_addr.sin_port)));

        // 处理客户端请求
        handleClient(client_socket);
    }
}

void HttpServer::handleClient(int client_socket) {
    // 读取客户端请求
    char buffer[1024];
    std::string request_str;
    ssize_t bytes_read;

    while ((bytes_read = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        request_str.append(buffer, bytes_read);

        // 检查是否收到完整的请求
        if (request_str.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }

    if (bytes_read == -1) {
        LOG_ERROR("Failed to read from client socket: " + std::string(strerror(errno)));
        close(client_socket);
        return;
    }

    if (request_str.empty()) {
        LOG_INFO("Client closed connection");
        close(client_socket);
        return;
    }

    // 解析HTTP请求
    HttpRequest request;
    if (!request.parse(request_str)) {
        LOG_ERROR("Failed to parse HTTP request");
        HttpResponse response;
        response.setStatusCode(HttpResponse::BAD_REQUEST);
        response.setBody("Bad Request");
        send(client_socket, response.toString().c_str(), response.toString().size(), 0);
        close(client_socket);
        return;
    }

    // 处理HTTP请求
    HttpResponse response;
    processRequest(request, response);

    // 发送HTTP响应
    if (send(client_socket, response.toString().c_str(), response.toString().size(), 0) == -1) {
        LOG_ERROR("Failed to send response to client: " + std::string(strerror(errno)));
    }

    // 关闭客户端套接字
    close(client_socket);
    LOG_INFO("Client connection closed");
}

void HttpServer::processRequest(const HttpRequest& request, HttpResponse& response) {
    // 设置默认响应头
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Server", "API Quota Server");

    // 查找处理程序
    HandlerKey key(request.getMethod(), request.getUri());
    auto it = handlers_.find(key);

    if (it != handlers_.end()) {
        // 调用处理程序
        try {
            it->second(request, response);
        } catch (const std::exception& e) {
            LOG_ERROR("Exception in request handler: " + std::string(e.what()));
            response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
            response.setBody("Internal Server Error");
        }
    } else {
        // 未找到处理程序
        LOG_ERROR("No handler found for request: " + request.getMethod() + " " + request.getUri());
        response.setStatusCode(HttpResponse::NOT_FOUND);
        response.setBody("Not Found");
    }
}
