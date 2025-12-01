#pragma once

#include <cstdint>
#include <string>

class TimeUtil {
public:
  // 获取当前时间戳（秒）
  static int64_t GetCurrentTimestamp();
  
  // 将时间戳转换为字符串（格式：YYYY-MM-DD HH:MM:SS）
  static std::string TimestampToString(int64_t timestamp);
  
  // 将字符串转换为时间戳（格式：YYYY-MM-DD HH:MM:SS）
  static int64_t StringToTimestamp(const std::string& time_str);
};
