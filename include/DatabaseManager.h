#pragma once

#include <sqlite3.h>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include "OrderBook.h"

class DatabaseManager {
private:
    std::string db_path_;
    sqlite3* db_ = nullptr;
    
    // 成交记录队列
    std::queue<Trade> trade_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    
    // 工作线程
    std::thread worker_thread_;
    std::atomic<bool> running_ = true;
    
    // 初始化数据库表
    bool initialize_tables();
    
    // 工作线程函数
    void worker();
    
    // 执行SQL语句
    bool execute_sql(const std::string& sql);
    
    // 将时间戳转换为SQLite格式
    std::string timestamp_to_sqlite(const std::chrono::system_clock::time_point& timestamp) const;

public:
    explicit DatabaseManager(std::string db_path);
    ~DatabaseManager();
    
    // 添加成交记录到队列
    void add_trade(const Trade& trade);
    
    // 批量添加成交记录
    void add_trades(const std::vector<Trade>& trades);
    
    // 查询成交记录
    std::vector<Trade> get_trades(const std::string& symbol, std::size_t limit = 100);
    
    // 关闭数据库
    void shutdown();
};
