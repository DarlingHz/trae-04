#include "utils/time.h"
#include <iomanip>
#include <sstream>

namespace utils {

uint64_t TimeUtils::getCurrentTimestamp() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

uint64_t TimeUtils::getCurrentTimestampMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

std::string TimeUtils::timestampToIsoString(uint64_t timestamp) {
    auto timePoint = std::chrono::system_clock::from_time_t(timestamp);
    auto tm = std::chrono::system_clock::to_time_t(timePoint);
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&tm), "%Y-%m-%dT%H:%M:%SZ");
    
    return oss.str();
}

bool TimeUtils::isExpired(uint64_t expireTimestamp) {
    if (expireTimestamp == 0) { // 永不过期
        return false;
    }
    
    return getCurrentTimestamp() > expireTimestamp;
}

} // namespace utils
