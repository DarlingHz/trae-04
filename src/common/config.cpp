#include "config.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace common {

Config::Config() {
    // 设置默认值
    setString("server.port", "8080");
    setString("server.host", "0.0.0.0");
    setString("database.host", "localhost");
    setString("database.port", "3306");
    setString("database.username", "root");
    setString("database.password", "");
    setString("database.name", "online_judge");
    setInt("database.pool_size", 10);
    setString("jwt.secret", "default_secret_key_change_in_production");
    setInt("jwt.expire_hours", 24);
    setString("log.level", "INFO");
}

Config::~Config() {
}

bool Config::loadFromFile(const std::string& configFile) {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << configFile << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // 跳过注释和空行
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        size_t delimiterPos = line.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);
            
            // 去除前后空格
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            configValues_[key] = value;
        }
    }

    file.close();
    return true;
}

std::string Config::getString(const std::string& key, const std::string& defaultValue) const {
    auto it = configValues_.find(key);
    if (it != configValues_.end()) {
        return it->second;
    }
    return defaultValue;
}

int Config::getInt(const std::string& key, int defaultValue) const {
    auto it = configValues_.find(key);
    if (it != configValues_.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            // 如果转换失败，返回默认值
        }
    }
    return defaultValue;
}

bool Config::getBool(const std::string& key, bool defaultValue) const {
    auto it = configValues_.find(key);
    if (it != configValues_.end()) {
        std::string value = it->second;
        if (value == "true" || value == "1" || value == "yes") {
            return true;
        } else if (value == "false" || value == "0" || value == "no") {
            return false;
        }
    }
    return defaultValue;
}

void Config::setString(const std::string& key, const std::string& value) {
    configValues_[key] = value;
}

void Config::setInt(const std::string& key, int value) {
    configValues_[key] = std::to_string(value);
}

void Config::setBool(const std::string& key, bool value) {
    configValues_[key] = value ? "true" : "false";
}

// 全局配置实例初始化
Config g_config;

} // namespace common
