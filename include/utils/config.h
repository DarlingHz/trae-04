#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace utils {

class Config {
public:
    static Config& getInstance();
    
    bool load(const std::string& configPath);
    
    int getHttpPort() const;
    std::string getDbPath() const;
    int getCacheSize() const;
    
private:
    Config();
    ~Config() = default;
    
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    json config_;
};

} // namespace utils
