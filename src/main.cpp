#include <crow.h>
#include <iostream>
#include <signal.h>
#include "Exchange.h"
#include "ApiHandler.h"

std::shared_ptr<Exchange> exchange;
std::unique_ptr<crow::SimpleApp> app;

void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    
    if (app) {
        app->stop();
    }
    
    if (exchange) {
        exchange->shutdown();
    }
    
    exit(0);
}

int main(int /*argc*/, char** /*argv*/) {
    try {
        // 初始化信号处理
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
        
        // 初始化交易所
        exchange = std::make_shared<Exchange>("./exchange.db");
        std::cout << "Exchange initialized successfully" << std::endl;
        
        // 初始化API处理程序
        ApiHandler api_handler(exchange);
        
        // 初始化Crow应用
        app = std::make_unique<crow::SimpleApp>();
        
        // 注册API路由
        api_handler.register_routes(*app);
        
        // 启动服务器
        uint16_t port = 8081;
        std::cout << "Starting server on port " << port << "..." << std::endl;
        app->port(port).multithreaded().run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
