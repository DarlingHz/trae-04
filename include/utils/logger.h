#pragma once

#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>

namespace utils {

enum class LogLevel {
    INFO,
    ERROR
};

class Logger {
public:
    static Logger& getInstance();
    
    void setLogLevel(LogLevel level);
    
    template<typename... Args>
    void info(const Args&... args) {
        if (currentLevel_ <= LogLevel::INFO) {
            log(LogLevel::INFO, args...);
        }
    }
    
    template<typename... Args>
    void error(const Args&... args) {
        if (currentLevel_ <= LogLevel::ERROR) {
            log(LogLevel::ERROR, args...);
        }
    }
    
private:
    Logger();
    ~Logger() = default;
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    template<typename... Args>
    void log(LogLevel level, const Args&... args) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        std::cout << " [" << (level == LogLevel::INFO ? "INFO" : "ERROR") << "] ";
        
        (std::cout << ... << args) << std::endl;
    }
    
    LogLevel currentLevel_;
    std::mutex mutex_;
};

#define LOG_INFO utils::Logger::getInstance().info
#define LOG_ERROR utils::Logger::getInstance().error

} // namespace utils
