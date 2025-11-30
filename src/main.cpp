#include "utils/config.h"
#include "utils/logger.h"
#include "storage/db_manager.h"
#include "http/handler.h"
#include <httplib.h>
#include <iostream>

int main() {
    try {
        // 初始化日志
        utils::Logger::getInstance().info("Starting short link service...");
        
        // 加载配置
        utils::Config& config = utils::Config::getInstance();
        if (!config.load("./config/config.json")) {
            utils::Logger::getInstance().error("Failed to load config file");
            return 1;
        }
        
        // 初始化数据库
        storage::DBManager& dbManager = storage::DBManager::getInstance();
        if (!dbManager.init(config.getDbPath())) {
            utils::Logger::getInstance().error("Failed to initialize database");
            return 1;
        }
        
        // 创建HTTP服务器
        httplib::Server server;
        
        // 设置线程池大小
        // server.set_thread_pool(std::make_shared<httplib::ThreadPool>(8)); // 可以根据配置调整
        
        // 注册路由
        http::Handler handler(config.getCacheSize());
        handler.registerRoutes(server);
        
        // 启动服务器
        int port = config.getHttpPort();
        utils::Logger::getInstance().info("Server started successfully on port ", port);
        
        if (!server.listen("0.0.0.0", port)) {
            utils::Logger::getInstance().error("Failed to start server on port ", port);
            return 1;
        }
        
    } catch (const std::exception& e) {
        utils::Logger::getInstance().error("Unhandled exception: ", e.what());
        return 1;
    }
    
    return 0;
}
