#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>

namespace common {

class Config {
public:
    Config();
    ~Config();

    // 从配置文件加载配置
    bool loadFromFile(const std::string& configFile);
    
    // 获取配置项
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    int getInt(const std::string& key, int defaultValue = 0) const;
    bool getBool(const std::string& key, bool defaultValue = false) const;
    
    // 设置配置项（用于测试或运行时修改）
    void setString(const std::string& key, const std::string& value);
    void setInt(const std::string& key, int value);
    void setBool(const std::string& key, bool value);

private:
    std::unordered_map<std::string, std::string> configValues_;
};

// 全局配置实例
extern Config g_config;

} // namespace common

#endif // CONFIG_H
