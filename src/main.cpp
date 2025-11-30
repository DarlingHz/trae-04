// 主函数源文件

#include <iostream>
#include <memory>
#include <stdexcept>

#include "config.h"
#include "database.h"
#include "auth_service.h"
#include "project_service.h"
#include "task_service.h"
#include "stats_service.h"
#include "audit_log_service.h"
#include "http_server.h"
#include "utils.h"

int main() {
    try {
        // 初始化数据库
        std::shared_ptr<Database> database = std::make_shared<Database>(config::kDatabasePath, config::kDatabaseConnectionPoolSize);
        
        // 初始化服务
        std::shared_ptr<AuthService> auth_service = std::make_shared<AuthService>(database);
        std::shared_ptr<ProjectService> project_service = std::make_shared<ProjectService>(database, auth_service);
        std::shared_ptr<TaskService> task_service = std::make_shared<TaskService>(database, auth_service);
        std::shared_ptr<StatsService> stats_service = std::make_shared<StatsService>(database, auth_service);
        std::shared_ptr<AuditLogService> audit_log_service = std::make_shared<AuditLogService>(database, auth_service);
        
        // 初始化IO上下文
        boost::asio::io_context io_context;
        
        // 初始化HTTP服务器
        HttpServer server(io_context, 8080, database, auth_service, task_service, project_service, stats_service, audit_log_service);
        
        // 启动服务器
        server.Start();
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to start server: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}