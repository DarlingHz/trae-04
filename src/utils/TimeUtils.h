#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <chrono>
#include <string>

namespace utils {

class TimeUtils {
public:
    TimeUtils() = default;
    ~TimeUtils() = default;

    // 将 system_clock::time_point 转换为 ISO 8601 格式字符串
    static std::string timePointToIsoString(const std::chrono::system_clock::time_point& tp);

    // 将 ISO 8601 格式字符串转换为 system_clock::time_point
    static std::chrono::system_clock::time_point isoStringToTimePoint(const std::string& str);

    // 将 system_clock::time_point 转换为 Unix 时间戳（秒）
    static int64_t timePointToUnixTimestamp(const std::chrono::system_clock::time_point& tp);

    // 将 Unix 时间戳（秒）转换为 system_clock::time_point
    static std::chrono::system_clock::time_point unixTimestampToTimePoint(int64_t timestamp);

    // 获取当前时间的 ISO 8601 格式字符串
    static std::string getCurrentTimeIsoString();

    // 获取当前时间的 Unix 时间戳（秒）
    static int64_t getCurrentUnixTimestamp();

private:
    TimeUtils(const TimeUtils&) = delete;
    TimeUtils& operator=(const TimeUtils&) = delete;
};

} // namespace utils

#endif // TIME_UTILS_H
