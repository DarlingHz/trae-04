#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "server.h"
#include "database.h"

int main(int argc, char* argv[]) {
    // 默认配置
    int port = 8080;
    std::string db_path = "inventory_order_management.db";
    
    // 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--port" || arg == "-p") {
            if (i + 1 < argc) {
                port = std::stoi(argv[i + 1]);
                i++;
            }
        } else if (arg == "--db" || arg == "-d") {
            if (i + 1 < argc) {
                db_path = argv[i + 1];
                i++;
            }
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "用法: inventory_order_management [选项]" << std::endl;
            std::cout << "选项:" << std::endl;
            std::cout << "  -p, --port <端口>    服务器监听端口 (默认: 8080)" << std::endl;
            std::cout << "  -d, --db <路径>       数据库文件路径 (默认: inventory_order_management.db)" << std::endl;
            std::cout << "  -h, --help             显示帮助信息" << std::endl;
            return 0;
        }
    }
    
    // 初始化数据库
    Database db;
    if (!db.open(db_path)) {
        std::cerr << "数据库初始化失败" << std::endl;
        return 1;
    }
    
    // 初始化服务器
    HttpServer server(port, db);
    
    // 启动服务器
    server.start();
    
    // 等待用户输入退出命令
    std::cout << "按 Ctrl+C 停止服务器" << std::endl;
    
    // 保持主线程运行
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}