#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <nlohmann/json.hpp>

namespace utils {

class Config {
public:
    Config() = default;
    ~Config() = default;

    // 加载配置文件
    bool load(const std::string& file_path);

    // HTTP 配置
    int getHttpPort() const;
    int getThreadPoolSize() const;

    // 数据库配置
    std::string getDbType() const;
    std::string getDbPath() const;
    std::string getDbHost() const;
    int getDbPort() const;
    std::string getDbName() const;
    std::string getDbUser() const;
    std::string getDbPassword() const;

    // 缓存配置
    int getCacheCapacity() const;
    int getCacheTtlSeconds() const;

    // 日志配置
    std::string getLogLevel() const;
    std::string getLogFilePath() const;

private:
    nlohmann::json config_;
};

// 全局配置实例
extern Config g_config;

} // namespace utils

#endif // CONFIG_H
