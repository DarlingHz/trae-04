#pragma once

#include <string>
#include <chrono>
#include <cstdint>

namespace utils {

class TimeUtils {
public:
    // 获取当前时间戳（秒）
    static uint64_t getCurrentTimestamp();
    
    // 获取当前时间戳（毫秒）
    static uint64_t getCurrentTimestampMs();
    
    // 将时间戳转换为ISO 8601格式的字符串
    static std::string timestampToIsoString(uint64_t timestamp);
    
    // 检查是否过期
    static bool isExpired(uint64_t expireTimestamp);
};

} // namespace utils
