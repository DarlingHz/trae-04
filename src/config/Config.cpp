#include "config/Config.hpp"
#include "utils/Utils.hpp"
#include "utils/Logger.hpp"
#include <fstream>
#include <sstream>

using namespace Utils;

bool Config::load(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        LOG_WARNING("Config file not found, using default values: " + config_file);
        return true;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json_str = buffer.str();

    try {
        JsonValue config_json = parseJson(json_str);

        if (config_json.hasKey("port")) {
            port_ = static_cast<uint16_t>(config_json["port"].asInt());
        }

        if (config_json.hasKey("db_path")) {
            db_path_ = config_json["db_path"].asString();
        }

        if (config_json.hasKey("thread_pool_size")) {
            thread_pool_size_ = static_cast<uint32_t>(config_json["thread_pool_size"].asInt());
        }

        if (config_json.hasKey("default_daily_quota")) {
            default_daily_quota_ = static_cast<uint32_t>(config_json["default_daily_quota"].asInt());
        }

        if (config_json.hasKey("default_per_minute_quota")) {
            default_per_minute_quota_ = static_cast<uint32_t>(config_json["default_per_minute_quota"].asInt());
        }

        LOG_INFO("Config loaded successfully from: " + config_file);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to parse config file: " + std::string(e.what()));
        return false;
    }
}
