#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>

namespace utils {

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger {
public:
    Logger() = default;
    ~Logger();

    // 初始化日志器
    bool init(const std::string& log_file_path, LogLevel level);

    // 日志方法
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);

    // 设置日志级别
    void setLogLevel(LogLevel level);

private:
    // 日志输出
    void log(LogLevel level, const std::string& message);

    // 获取当前时间字符串
    std::string getCurrentTimeString() const;

    // 日志级别转换为字符串
    std::string logLevelToString(LogLevel level) const;

private:
    std::ofstream log_file_;
    LogLevel log_level_ = LogLevel::INFO;
    std::mutex mutex_;
};

// 全局日志器实例
extern Logger g_logger;

} // namespace utils

// 日志宏
#define LOG_DEBUG(msg) utils::g_logger.debug(msg)
#define LOG_INFO(msg) utils::g_logger.info(msg)
#define LOG_WARN(msg) utils::g_logger.warn(msg)
#define LOG_ERROR(msg) utils::g_logger.error(msg)

#endif // LOGGER_H
