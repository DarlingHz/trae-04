#include "Config.h"
#include <fstream>
#include <iostream>

namespace utils {

Config g_config;

bool Config::load(const std::string& file_path) {
    std::ifstream ifs(file_path);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open config file: " << file_path << std::endl;
        return false;
    }

    try {
        ifs >> config_;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Failed to parse config file: " << e.what() << std::endl;
        return false;
    }

    return true;
}

int Config::getHttpPort() const {
    return config_.value("http.port", 8080);
}

int Config::getThreadPoolSize() const {
    return config_.value("http.thread_pool_size", 10);
}

std::string Config::getDbType() const {
    return config_.value("database.type", "sqlite");
}

std::string Config::getDbPath() const {
    return config_.value("database.path", "./db/watch_server.db");
}

std::string Config::getDbHost() const {
    return config_.value("database.host", "localhost");
}

int Config::getDbPort() const {
    return config_.value("database.port", 3306);
}

std::string Config::getDbName() const {
    return config_.value("database.name", "watch_server");
}

std::string Config::getDbUser() const {
    return config_.value("database.user", "root");
}

std::string Config::getDbPassword() const {
    return config_.value("database.password", "");
}

int Config::getCacheCapacity() const {
    return config_.value("cache.capacity", 100);
}

int Config::getCacheTtlSeconds() const {
    return config_.value("cache.ttl_seconds", 300);
}

std::string Config::getLogLevel() const {
    return config_.value("logging.level", "info");
}

std::string Config::getLogFilePath() const {
    return config_.value("logging.file_path", "./logs/server.log");
}

} // namespace utils
