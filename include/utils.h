// 工具函数头文件

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <random>

// 字符串工具函数
namespace string_utils {
    
    // 修剪字符串开头的空白字符
    std::string TrimLeft(const std::string& str);
    
    // 修剪字符串结尾的空白字符
    std::string TrimRight(const std::string& str);
    
    // 修剪字符串开头和结尾的空白字符
    std::string Trim(const std::string& str);
    
    // 将字符串转换为小写
    std::string ToLower(const std::string& str);
    
    // 将字符串转换为大写
    std::string ToUpper(const std::string& str);
    
    // 检查字符串是否为空或仅包含空白字符
    bool IsEmptyOrWhitespace(const std::string& str);
    
    // 检查字符串是否为有效的邮箱格式
    bool IsValidEmail(const std::string& str);
    
    // 分割字符串
    std::vector<std::string> Split(const std::string& str, char delimiter);
    
    // 连接字符串
    std::string Join(const std::vector<std::string>& parts, const std::string& separator);
}

// 时间工具函数
namespace time_utils {
    
    // 将系统时间点转换为ISO 8601格式的字符串
    std::string ToIsoString(const std::chrono::system_clock::time_point& time_point);
    
    // 将ISO 8601格式的字符串转换为系统时间点
    std::optional<std::chrono::system_clock::time_point> FromIsoString(const std::string& iso_string);
    
    // 获取当前系统时间点
    std::chrono::system_clock::time_point Now();
    
    // 将系统时间点转换为Unix时间戳（秒）
    std::time_t ToUnixTimestamp(const std::chrono::system_clock::time_point& time_point);
    
    // 将Unix时间戳（秒）转换为系统时间点
    std::chrono::system_clock::time_point FromUnixTimestamp(std::time_t timestamp);
}

// 加密工具函数
namespace crypto_utils {
    
    // 计算字符串的SHA256哈希值
    std::string Sha256(const std::string& str);
    
    // 生成随机字符串
    std::string GenerateRandomString(int length);
}

// URL工具函数
namespace url_utils {
    
    // URL编码
    std::string UrlEncode(const std::string& str);
    
    // URL解码
    std::string UrlDecode(const std::string& str);
    
    // 解析查询参数
    std::unordered_map<std::string, std::string> ParseQueryParams(const std::string& query_string);
}

#endif  // UTILS_H