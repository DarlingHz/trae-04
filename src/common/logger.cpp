#include "logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace common {

Logger::Logger() : currentLevel_(LogLevel::INFO), useStdout_(true) {
}

Logger::~Logger() {
    if (logFileStream_.is_open()) {
        logFileStream_.close();
    }
}

void Logger::setLogLevel(LogLevel level) {
    currentLevel_ = level;
}

bool Logger::setLogFile(const std::string& logFile) {
    if (logFileStream_.is_open()) {
        logFileStream_.close();
    }

    logFileStream_.open(logFile, std::ios::out | std::ios::app);
    if (!logFileStream_.is_open()) {
        std::cerr << "Failed to open log file: " << logFile << std::endl;
        return false;
    }

    useStdout_ = false;
    return true;
}

void Logger::debug(const std::string& message) {
    if (LogLevel::DEBUG >= currentLevel_) {
        log(LogLevel::DEBUG, message);
    }
}

void Logger::info(const std::string& message) {
    if (LogLevel::INFO >= currentLevel_) {
        log(LogLevel::INFO, message);
    }
}

void Logger::warning(const std::string& message) {
    if (LogLevel::WARNING >= currentLevel_) {
        log(LogLevel::WARNING, message);
    }
}

void Logger::error(const std::string& message) {
    if (LogLevel::ERROR >= currentLevel_) {
        log(LogLevel::ERROR, message);
    }
}

void Logger::fatal(const std::string& message) {
    if (LogLevel::FATAL >= currentLevel_) {
        log(LogLevel::FATAL, message);
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex_);

    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm local_tm;
#ifdef _WIN32
    localtime_s(&local_tm, &now_time);
#else
    localtime_r(&now_time, &local_tm);
#endif

    std::stringstream timeStream;
    timeStream << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") 
               << "." << std::setw(3) << std::setfill('0') << milliseconds.count();

    // 日志级别字符串
    std::string levelStr;
    switch (level) {
        case LogLevel::DEBUG:   levelStr = "DEBUG";
 break;
        case LogLevel::INFO:    levelStr = "INFO";
 break;
        case LogLevel::WARNING: levelStr = "WARNING";
 break;
        case LogLevel::ERROR:   levelStr = "ERROR";
 break;
        case LogLevel::FATAL:   levelStr = "FATAL";
 break;
        default:               levelStr = "UNKNOWN";
 break;
    }

    // 构建日志消息
    std::string logMessage = "[" + timeStream.str() + "] [" + levelStr + "] " + message;

    // 输出日志
    if (useStdout_) {
        if (level >= LogLevel::ERROR) {
            std::cerr << logMessage << std::endl;
        } else {
            std::cout << logMessage << std::endl;
        }
    } else {
        logFileStream_ << logMessage << std::endl;
        logFileStream_.flush();
    }
}

// 全局日志实例初始化
Logger g_logger;

} // namespace common
