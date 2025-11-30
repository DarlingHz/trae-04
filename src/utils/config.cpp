#include "utils/config.h"
#include "utils/logger.h"
#include <fstream>

namespace utils {

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

Config::Config() {
}

bool Config::load(const std::string& configPath) {
    try {
        std::ifstream configFile(configPath);
        if (!configFile.is_open()) {
            LOG_ERROR("Failed to open config file: ", configPath);
            return false;
        }
        
        configFile >> config_;
        
        // 验证必要的配置项
        if (!config_.contains("http_port") || !config_["http_port"].is_number_integer()) {
            LOG_ERROR("Invalid or missing http_port in config");
            return false;
        }
        
        if (!config_.contains("db_path") || !config_["db_path"].is_string()) {
            LOG_ERROR("Invalid or missing db_path in config");
            return false;
        }
        
        if (!config_.contains("cache_size") || !config_["cache_size"].is_number_integer()) {
            LOG_ERROR("Invalid or missing cache_size in config");
            return false;
        }
        
        LOG_INFO("Config loaded successfully from: ", configPath);
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load config: ", e.what());
        return false;
    }
}

int Config::getHttpPort() const {
    return config_["http_port"].get<int>();
}

std::string Config::getDbPath() const {
    return config_["db_path"].get<std::string>();
}

int Config::getCacheSize() const {
    return config_["cache_size"].get<int>();
}

} // namespace utils
