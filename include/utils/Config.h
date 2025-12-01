#ifndef RIDE_SHARING_BACKEND_CONFIG_H
#define RIDE_SHARING_BACKEND_CONFIG_H

#include <string>
#include <map>

namespace utils {

class Config {
public:
    ~Config();

    // 单例模式
    static Config& get_instance();

    // 加载配置文件
    bool load(const std::string& config_path);

    // 获取配置项
    std::string get_string(const std::string& key, const std::string& default_value = "") const;
    int get_int(const std::string& key, int default_value = 0) const;
    float get_float(const std::string& key, float default_value = 0.0f) const;
    bool get_bool(const std::string& key, bool default_value = false) const;

private:
    Config();
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    std::map<std::string, std::string> configs_;
};

} // namespace utils

#endif //RIDE_SHARING_BACKEND_CONFIG_H
