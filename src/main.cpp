#include "App.hpp"
#include "utils/Logger.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        // 检查命令行参数
        std::string config_file_path = "config.json"; // 默认配置文件路径
        if (argc > 1) {
            config_file_path = argv[1];
        }

        // 创建应用程序实例
        App app(config_file_path);

        // 初始化应用程序
        if (!app.init()) {
            std::cerr << "Failed to initialize application" << std::endl;
            return 1;
        }

        // 运行应用程序
        if (!app.run()) {
            std::cerr << "Failed to run application" << std::endl;
            return 1;
        }

        // 停止应用程序
        app.stop();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception in main: " << e.what() << std::endl;
        LOG_ERROR("Exception in main: " + std::string(e.what()));
        return 1;
    }
}
