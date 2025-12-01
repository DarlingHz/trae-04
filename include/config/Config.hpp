#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <cstdint>

class Config {
public:
    Config() = default;
    ~Config() = default;

    bool load(const std::string& config_file);

    // 获取配置项
    uint16_t getPort() const { return port_; }
    const std::string& getDbPath() const { return db_path_; }
    uint32_t getThreadPoolSize() const { return thread_pool_size_; }
    uint32_t getDefaultDailyQuota() const { return default_daily_quota_; }
    uint32_t getDefaultPerMinuteQuota() const { return default_per_minute_quota_; }

private:
    uint16_t port_ = 8080;
    std::string db_path_ = "./api_quota.db";
    uint32_t thread_pool_size_ = 10;
    uint32_t default_daily_quota_ = 10000;
    uint32_t default_per_minute_quota_ = 200;
};

#endif // CONFIG_HPP
