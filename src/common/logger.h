#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <cstdio>
#include <memory>

namespace common {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

class Logger {
public:
    Logger();
    ~Logger();

    // 设置日志级别
    void setLogLevel(LogLevel level);
    
    // 设置日志文件（可选，默认为标准输出）
    bool setLogFile(const std::string& logFile);
    
    // 日志记录函数
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);

    // 格式化的日志记录函数
    template<typename... Args>
    void debug(const char* format, Args... args);
    
    template<typename... Args>
    void info(const char* format, Args... args);
    
    template<typename... Args>
    void warning(const char* format, Args... args);
    
    template<typename... Args>
    void error(const char* format, Args... args);
    
    template<typename... Args>
    void fatal(const char* format, Args... args);

private:
    // 实际的日志记录函数
    void log(LogLevel level, const std::string& message);
    
    // 格式化消息
    template<typename... Args>
    std::string formatMessage(const char* format, Args... args);

private:
    LogLevel currentLevel_;
    std::ofstream logFileStream_;
    std::mutex logMutex_;
    bool useStdout_;
};

// 全局日志实例
extern Logger g_logger;

// 实现模板方法
template<typename... Args>
void Logger::debug(const char* format, Args... args) {
    if (LogLevel::DEBUG >= currentLevel_) {
        log(LogLevel::DEBUG, formatMessage(format, args...));
    }
}

template<typename... Args>
void Logger::info(const char* format, Args... args) {
    if (LogLevel::INFO >= currentLevel_) {
        log(LogLevel::INFO, formatMessage(format, args...));
    }
}

template<typename... Args>
void Logger::warning(const char* format, Args... args) {
    if (LogLevel::WARNING >= currentLevel_) {
        log(LogLevel::WARNING, formatMessage(format, args...));
    }
}

template<typename... Args>
void Logger::error(const char* format, Args... args) {
    if (LogLevel::ERROR >= currentLevel_) {
        log(LogLevel::ERROR, formatMessage(format, args...));
    }
}

template<typename... Args>
void Logger::fatal(const char* format, Args... args) {
    if (LogLevel::FATAL >= currentLevel_) {
        log(LogLevel::FATAL, formatMessage(format, args...));
    }
}

template<typename... Args>
std::string Logger::formatMessage(const char* format, Args... args) {
    // 计算所需的缓冲区大小
    size_t size = std::snprintf(nullptr, 0, format, args...) + 1; // +1 for null terminator
    if (size <= 0) {
        return "Format error";
    }
    
    // 分配缓冲区
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format, args...);
    
    return std::string(buf.get(), buf.get() + size - 1); // 不包含null终止符
}

} // namespace common

#endif // LOGGER_H
