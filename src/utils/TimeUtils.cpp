#include "TimeUtils.h"
#include <iomanip>
#include <sstream>
#include <ctime>

namespace utils {

std::string TimeUtils::timePointToIsoString(const std::chrono::system_clock::time_point& tp) {
    auto time_t_val = std::chrono::system_clock::to_time_t(tp);
    auto tm_val = *std::localtime(&time_t_val);
    
    std::stringstream ss;
    ss << std::put_time(&tm_val, "%Y-%m-%dT%H:%M:%S");
    
    // 添加毫秒
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()) % 1000;
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    // 添加时区
    ss << "+00:00"; // 暂时使用 UTC 时区
    
    return ss.str();
}

std::chrono::system_clock::time_point TimeUtils::isoStringToTimePoint(const std::string& str) {
    // 解析 ISO 8601 格式字符串
    std::tm tm_val = {};
    std::stringstream ss(str);
    
    ss >> std::get_time(&tm_val, "%Y-%m-%dT%H:%M:%S");
    if (ss.fail()) {
        throw std::invalid_argument("Invalid ISO 8601 date format");
    }
    
    // 解析毫秒
    int ms = 0;
    if (ss.peek() == '.') {
        ss.ignore();
        std::string ms_str;
        std::getline(ss, ms_str, '+');
        if (ms_str.length() > 3) {
            ms_str = ms_str.substr(0, 3);
        } else if (ms_str.length() < 3) {
            ms_str += std::string(3 - ms_str.length(), '0');
        }
        ms = std::stoi(ms_str);
    }
    
    // 转换为 time_point
    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm_val));
    tp += std::chrono::milliseconds(ms);
    
    return tp;
}

int64_t TimeUtils::timePointToUnixTimestamp(const std::chrono::system_clock::time_point& tp) {
    return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
}

std::chrono::system_clock::time_point TimeUtils::unixTimestampToTimePoint(int64_t timestamp) {
    return std::chrono::system_clock::from_time_t(static_cast<std::time_t>(timestamp));
}

std::string TimeUtils::getCurrentTimeIsoString() {
    return timePointToIsoString(std::chrono::system_clock::now());
}

int64_t TimeUtils::getCurrentUnixTimestamp() {
    return timePointToUnixTimestamp(std::chrono::system_clock::now());
}

} // namespace utils
