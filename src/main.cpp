#include <cpp-httplib/httplib.h>
#include <memory>
#include <iostream>
#include "utils/Logger.h"
#include "utils/Config.h"
#include "utils/DbConnectionPool.h"
#include "repository/UserRepository.h"
#include "repository/MovieRepository.h"
#include "repository/WatchRecordRepository.h"
#include "service/UserService.h"
#include "service/MovieService.h"
#include "service/WatchRecordService.h"
#include "service/StatsService.h"
#include "controller/UserController.h"
#include "controller/MovieController.h"
#include "controller/WatchRecordController.h"
#include "controller/StatsController.h"

int main() {
    try {
        // 加载配置文件
        utils::Config config;
        if (!config.load("config/config.json")) {
            std::cerr << "Failed to load config file" << std::endl;
            return 1;
        }
        
        // 初始化日志系统
        std::string log_file_path = config.getLogFilePath();
        std::string log_level_str = config.getLogLevel();
        utils::LogLevel log_level = utils::LogLevel::INFO;
        if (log_level_str == "DEBUG") {
            log_level = utils::LogLevel::DEBUG;
        } else if (log_level_str == "WARN") {
            log_level = utils::LogLevel::WARN;
        } else if (log_level_str == "ERROR") {
            log_level = utils::LogLevel::ERROR;
        }
        if (!utils::g_logger.init(log_file_path, log_level)) {
            std::cerr << "Failed to initialize logger" << std::endl;
            return 1;
        }
        LOG_INFO("Starting watch server...");
        
        // 初始化数据库连接池
        int db_pool_size = 5; // 使用默认值，因为Config类没有get方法
        std::string db_path = config.getDbPath();
        
        if (!utils::g_db_pool.init(db_path, db_pool_size)) {
            LOG_ERROR("Failed to initialize database connection pool");
            return 1;
        }
        LOG_INFO("Database connection pool initialized with size: " + std::to_string(db_pool_size));
        
        // 初始化缓存
        int cache_capacity = config.getCacheCapacity();
        int cache_ttl = config.getCacheTtlSeconds();
        
        utils::g_cache.init(cache_capacity, cache_ttl);
        LOG_INFO("Cache initialized with capacity: " + std::to_string(cache_capacity) + ", TTL: " + std::to_string(cache_ttl) + " seconds");
        
        // 创建数据访问层实例
        auto user_repo = std::make_shared<repository::UserRepository>();
        auto movie_repo = std::make_shared<repository::MovieRepository>();
        auto watch_record_repo = std::make_shared<repository::WatchRecordRepository>();
        
        // 创建服务层实例
        auto user_service = std::make_shared<service::UserService>(user_repo);
        auto movie_service = std::make_shared<service::MovieService>(movie_repo);
        auto watch_record_service = std::make_shared<service::WatchRecordService>(watch_record_repo, user_repo, movie_repo);
        auto stats_service = std::make_shared<service::StatsService>(watch_record_repo, movie_repo);
        
        // 创建控制器实例
        auto user_controller = std::make_shared<controller::UserController>(user_service);
        auto movie_controller = std::make_shared<controller::MovieController>(movie_service);
        auto watch_record_controller = std::make_shared<controller::WatchRecordController>(watch_record_service);
        auto stats_controller = std::make_shared<controller::StatsController>(stats_service);
        
        // 创建 HTTP 服务器
        int port = config.getHttpPort();
        httplib::Server svr;
        
        // 线程池大小设置已移除，因为httplib::Server不支持此方法
        
        // 注册路由
        
        // 用户管理路由
        svr.Post("/users", [user_controller](const httplib::Request& req, httplib::Response& res) {
            user_controller->createUser(req, res);
        });
        
        svr.Get("/users/(\\d+)", [user_controller](const httplib::Request& req, httplib::Response& res) {
            user_controller->getUserById(req, res);
        });
        
        // 影片管理路由
        svr.Post("/movies", [movie_controller](const httplib::Request& req, httplib::Response& res) {
            movie_controller->createMovie(req, res);
        });
        
        svr.Get("/movies/(\\d+)", [movie_controller](const httplib::Request& req, httplib::Response& res) {
            movie_controller->getMovieById(req, res);
        });
        
        svr.Get("/movies", [movie_controller](const httplib::Request& req, httplib::Response& res) {
            movie_controller->getMovies(req, res);
        });
        
        svr.Put("/movies/(\\d+)", [movie_controller](const httplib::Request& req, httplib::Response& res) {
            movie_controller->updateMovie(req, res);
        });
        
        svr.Delete("/movies/(\\d+)", [movie_controller](const httplib::Request& req, httplib::Response& res) {
            movie_controller->deleteMovie(req, res);
        });
        
        // 观影记录路由
        svr.Post("/watch_records", [watch_record_controller](const httplib::Request& req, httplib::Response& res) {
            watch_record_controller->createWatchRecord(req, res);
        });
        
        svr.Get("/users/(\\d+)/watch_records", [watch_record_controller](const httplib::Request& req, httplib::Response& res) {
            watch_record_controller->getWatchRecordsByUserId(req, res);
        });
        
        // 统计与推荐路由
        svr.Get("/users/(\\d+)/stats/summary", [stats_controller](const httplib::Request& req, httplib::Response& res) {
            stats_controller->getUserStatsSummary(req, res);
        });
        
        svr.Get("/users/(\\d+)/stats/recommendations", [stats_controller](const httplib::Request& req, httplib::Response& res) {
            stats_controller->getUserRecommendations(req, res);
        });
        
        // 启动服务器
        LOG_INFO("Starting HTTP server on port: " + std::to_string(port));
        if (svr.listen("0.0.0.0", port)) {
            LOG_INFO("Server started successfully");
        } else {
            LOG_ERROR("Failed to start server on port: " + std::to_string(port));
            return 1;
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Unexpected error during server initialization: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}
