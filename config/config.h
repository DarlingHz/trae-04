// 配置文件

#ifndef CONFIG_H
#define CONFIG_H

#include <string>

// 服务配置
constexpr int SERVER_PORT = 8080;
constexpr int SERVER_THREAD_POOL_SIZE = 4;

// 数据库配置
constexpr const char* DATABASE_PATH = "./data/task_manager.db";
constexpr int DATABASE_POOL_SIZE = 10;

// 认证配置
constexpr const char* AUTH_TOKEN_SECRET = "your_secret_key_here";
constexpr int AUTH_TOKEN_EXPIRY_HOURS = 24;

// 日志配置
constexpr const char* LOG_FILE_PATH = "./logs/task_manager.log";
constexpr int LOG_MAX_FILE_SIZE = 10 * 1024 * 1024;  // 10MB
constexpr int LOG_MAX_BACKUP_FILES = 5;

// 缓存配置
constexpr int CACHE_USER_EXPIRY_SECONDS = 600;  // 10分钟
constexpr int CACHE_PROJECT_EXPIRY_SECONDS = 300;  // 5分钟

// API配置
constexpr const char* API_VERSION = "v1";
constexpr const char* API_PREFIX = "/api/";

// 分页配置
constexpr int DEFAULT_PAGE_SIZE = 10;
constexpr int MAX_PAGE_SIZE = 100;

#endif // CONFIG_H