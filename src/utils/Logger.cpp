#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace utils {

Logger g_logger;

Logger::~Logger() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

bool Logger::init(const std::string& log_file_path, LogLevel level) {
    // 打开日志文件
    log_file_.open(log_file_path, std::ios::app);
    if (!log_file_.is_open()) {
        std::cerr << "Failed to open log file: " << log_file_path << std::endl;
        return false;
    }

    log_level_ = level;
    info("Logger initialized");
    return true;
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warn(const std::string& message) {
    log(LogLevel::WARN, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    log_level_ = level;
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (level < log_level_) {
        return;
    }

    std::string time_str = getCurrentTimeString();
    std::string level_str = logLevelToString(level);
    std::string log_message = time_str + " [" + level_str + "] " + message;

    // 输出到标准输出
    std::cout << log_message << std::endl;

    // 输出到日志文件
    if (log_file_.is_open()) {
        log_file_ << log_message << std::endl;
        log_file_.flush();
    }
}

std::string Logger::getCurrentTimeString() const {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S") 
       << "." << std::setfill('0') << std::setw(3) << ms.count();

    return ss.str();
}

std::string Logger::logLevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARN:
            return "WARN";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

} // namespace utils
