#include "utils/Config.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace utils {

Config::Config() {
}

Config::~Config() {
}

Config& Config::get_instance() {
    static Config instance;
    return instance;
}

bool Config::load(const std::string& config_path) {
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        std::cerr << "无法打开配置文件: " << config_path << std::endl;
        return false;
    }

    configs_.clear();

    std::string line;
    while (std::getline(config_file, line)) {
        // 去除注释和空白符
        size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }

        // 去除前后空白符
        size_t start_pos = line.find_first_not_of(" \t\r\n");
        if (start_pos == std::string::npos) {
            // 空行
            continue;
        }

        size_t end_pos = line.find_last_not_of(" \t\r\n");
        line = line.substr(start_pos, end_pos - start_pos + 1);

        // 解析键值对
        size_t equal_pos = line.find('=');
        if (equal_pos == std::string::npos) {
            // 无效的配置项
            std::cerr << "无效的配置项: " << line << std::endl;
            continue;
        }

        std::string key = line.substr(0, equal_pos);
        std::string value = line.substr(equal_pos + 1);

        // 去除键值对的前后空白符
        size_t key_start = key.find_first_not_of(" \t");
        size_t key_end = key.find_last_not_of(" \t");
        if (key_start != std::string::npos && key_end != std::string::npos) {
            key = key.substr(key_start, key_end - key_start + 1);
        }

        size_t value_start = value.find_first_not_of(" \t");
        size_t value_end = value.find_last_not_of(" \t");
        if (value_start != std::string::npos && value_end != std::string::npos) {
            value = value.substr(value_start, value_end - value_start + 1);
        }

        configs_[key] = value;
    }

    config_file.close();
    return true;
}

std::string Config::get_string(const std::string& key, const std::string& default_value) const {
    auto it = configs_.find(key);
    if (it != configs_.end()) {
        return it->second;
    }
    return default_value;
}

int Config::get_int(const std::string& key, int default_value) const {
    auto it = configs_.find(key);
    if (it != configs_.end()) {
        try {
            return std::stoi(it->second);
        } catch (const std::exception& e) {
            std::cerr << "配置项转换为整数失败: " << key << " = " << it->second << std::endl;
        }
    }
    return default_value;
}

float Config::get_float(const std::string& key, float default_value) const {
    auto it = configs_.find(key);
    if (it != configs_.end()) {
        try {
            return std::stof(it->second);
        } catch (const std::exception& e) {
            std::cerr << "配置项转换为浮点数失败: " << key << " = " << it->second << std::endl;
        }
    }
    return default_value;
}

bool Config::get_bool(const std::string& key, bool default_value) const {
    auto it = configs_.find(key);
    if (it != configs_.end()) {
        std::string value = it->second;
        if (value == "true" || value == "1" || value == "yes") {
            return true;
        } else if (value == "false" || value == "0" || value == "no") {
            return false;
        }
    }
    return default_value;
}

} // namespace utils
