#include "utils/Logger.hpp"
#include "utils/Utils.hpp"
#include <iostream>

using namespace Utils;

Logger::Logger() : log_level_(INFO) {
}

Logger::~Logger() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

bool Logger::init(const std::string& log_file, Level level) {
    log_file_.open(log_file, std::ios::app);
    if (!log_file_.is_open()) {
        std::cerr << "Failed to open log file: " << log_file << std::endl;
        return false;
    }

    log_level_ = level;
    return true;
}

void Logger::setLevel(Level level) {
    log_level_ = level;
}

void Logger::debug(const std::string& message) {
    log(DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(INFO, message);
}

void Logger::warning(const std::string& message) {
    log(WARNING, message);
}

void Logger::error(const std::string& message) {
    log(ERROR, message);
}

void Logger::fatal(const std::string& message) {
    log(FATAL, message);
}

std::string Logger::levelToString(Level level) const {
    switch (level) {
        case DEBUG:
            return "DEBUG";
        case INFO:
            return "INFO";
        case WARNING:
            return "WARNING";
        case ERROR:
            return "ERROR";
        case FATAL:
            return "FATAL";
        default:
            return "UNKNOWN";
    }
}

void Logger::log(Level level, const std::string& message) {
    if (level < log_level_) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    std::string log_line = getCurrentTimeStr() + " [" + levelToString(level) + "] " + message;

    if (log_file_.is_open()) {
        log_file_ << log_line << std::endl;
    }

    // 错误和致命日志也输出到标准错误
    if (level >= ERROR) {
        std::cerr << log_line << std::endl;
    } else {
        std::cout << log_line << std::endl;
    }
}
