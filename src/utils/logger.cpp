#include "utils/logger.h"

namespace utils {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : currentLevel_(LogLevel::INFO) {
}

void Logger::setLogLevel(LogLevel level) {
    currentLevel_ = level;
}

} // namespace utils
