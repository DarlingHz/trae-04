// 配置文件头文件

#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace config {

// 服务配置
constexpr int kServerPort = 8080;
constexpr int kThreadPoolSize = 4;

// 数据库配置
const std::string kDatabasePath = "/Users/soma/code/trae/11-30-2/04/team_task_manager.db";
constexpr int kDatabaseConnectionPoolSize = 10;

// 缓存配置
constexpr int kUserCacheExpirationSeconds = 300;  // 5分钟
constexpr int kProjectCacheExpirationSeconds = 600;  // 10分钟

// 认证配置
constexpr int kAccessTokenExpirationDays = 7;
const std::string kAccessTokenSecret = "team_task_manager_secret_key";  // 生产环境中应从配置文件读取

// 日志配置
const std::string kLogFilePath = "task_manager.log";
constexpr int kLogFileMaxSize = 10 * 1024 * 1024;  // 10MB
constexpr int kLogFileMaxBackupCount = 5;

// API配置
const std::string kApiVersion = "v1";
const std::string kApiPrefix = "/api/" + kApiVersion;

// 分页配置
constexpr int kDefaultPageSize = 10;
constexpr int kMaxPageSize = 100;

}  // namespace config

#endif  // CONFIG_H