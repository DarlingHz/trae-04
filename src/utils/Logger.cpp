#include "utils/Logger.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

namespace utils {

Logger::Logger() {
}

Logger::~Logger() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

Logger& Logger::get_instance() {
    static Logger instance;
    return instance;
}

bool Logger::init(const std::string& log_path, LogLevel level) {
    if (log_file_.is_open()) {
        // 已经初始化过了
        return true;
    }

    log_file_.open(log_path, std::ios::app);
    if (!log_file_.is_open()) {
        std::cerr << "无法打开日志文件: " << log_path << std::endl;
        return false;
    }

    log_level_ = level;
    info("日志系统初始化完成");

    return true;
}

void Logger::set_level(LogLevel level) {
    log_level_ = level;
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

// LogStream类的实现
Logger::LogStream::LogStream(Logger& logger, LogLevel level)
    : logger_(logger), level_(level) {
}

Logger::LogStream::~LogStream() {
    // 在析构函数中输出日志
    switch (level_) {
        case LogLevel::DEBUG:
            logger_.debug(buffer_.str());
            break;
        case LogLevel::INFO:
            logger_.info(buffer_.str());
            break;
        case LogLevel::WARNING:
            logger_.warning(buffer_.str());
            break;
        case LogLevel::ERROR:
            logger_.error(buffer_.str());
            break;
    }
}

// Logger类的流式日志输出方法实现
Logger::LogStream Logger::debug() {
    return LogStream(*this, LogLevel::DEBUG);
}

Logger::LogStream Logger::info() {
    return LogStream(*this, LogLevel::INFO);
}

Logger::LogStream Logger::warning() {
    return LogStream(*this, LogLevel::WARNING);
}

Logger::LogStream Logger::error() {
    return LogStream(*this, LogLevel::ERROR);
}

std::string Logger::format_message(LogLevel level, const std::string& message) {
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto tm_now = std::localtime(&time_t_now);

    // 格式化时间
    std::stringstream time_ss;
    time_ss << std::put_time(tm_now, "%Y-%m-%d %H:%M:%S");

    // 日志级别字符串
    std::string level_str;
    switch (level) {
        case LogLevel::DEBUG:
            level_str = "DEBUG";
            break;
        case LogLevel::INFO:
            level_str = "INFO";
            break;
        case LogLevel::WARNING:
            level_str = "WARNING";
            break;
        case LogLevel::ERROR:
            level_str = "ERROR";
            break;
    }

    // 格式化消息
    return time_ss.str() + " [" + level_str + "] " + message;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < log_level_) {
        // 日志级别不够，不输出
        return;
    }

    std::string formatted_message = format_message(level, message);

    // 线程安全的日志输出
    std::lock_guard<std::mutex> lock(log_mutex_);

    // 输出到文件
    if (log_file_.is_open()) {
        log_file_ << formatted_message << std::endl;
    }

    // 同时输出到控制台
    if (level >= LogLevel::WARNING) {
        std::cerr << formatted_message << std::endl;
    } else {
        std::cout << formatted_message << std::endl;
    }
}

} // namespace utils
