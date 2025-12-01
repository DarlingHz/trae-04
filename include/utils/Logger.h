#ifndef RIDE_SHARING_BACKEND_LOGGER_H
#define RIDE_SHARING_BACKEND_LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <sstream>

namespace utils {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    ~Logger();

    // 单例模式
    static Logger& get_instance();

    // 初始化日志
    bool init(const std::string& log_path, LogLevel level = LogLevel::INFO);

    // 设置日志级别
    void set_level(LogLevel level);

    // 日志输出方法
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);

    // 流式日志输出方法
    class LogStream {
    public:
        LogStream(Logger& logger, LogLevel level);
        ~LogStream();

        // 重载<<运算符，支持不同类型的数据
        template<typename T>
        LogStream& operator<<(const T& value) {
            buffer_ << value;
            return *this;
        }

    private:
        Logger& logger_;
        LogLevel level_;
        std::ostringstream buffer_;
    };

    LogStream debug();
    LogStream info();
    LogStream warning();
    LogStream error();

private:
    Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // 格式化日志消息
    std::string format_message(LogLevel level, const std::string& message);

    // 输出日志
    void log(LogLevel level, const std::string& message);

    std::ofstream log_file_;
    LogLevel log_level_ = LogLevel::INFO;
    std::mutex log_mutex_;
};

// 日志宏定义
#define LOG_DEBUG utils::Logger::get_instance().debug()
#define LOG_INFO utils::Logger::get_instance().info()
#define LOG_WARNING utils::Logger::get_instance().warning()
#define LOG_ERROR utils::Logger::get_instance().error()

} // namespace utils

#endif //RIDE_SHARING_BACKEND_LOGGER_H
