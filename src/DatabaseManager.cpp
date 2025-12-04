#include "DatabaseManager.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>

DatabaseManager::DatabaseManager(std::string db_path) : db_path_(std::move(db_path)), running_(true) {
    // 打开数据库
    int rc = sqlite3_open(db_path_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::string error = sqlite3_errmsg(db_);
        sqlite3_close(db_);
        throw std::runtime_error("Failed to open database: " + error);
    }
    
    // 初始化表
    if (!initialize_tables()) {
        sqlite3_close(db_);
        throw std::runtime_error("Failed to initialize database tables");
    }
    
    // 启动工作线程
    worker_thread_ = std::thread(&DatabaseManager::worker, this);
}

DatabaseManager::~DatabaseManager() {
    shutdown();
}

bool DatabaseManager::initialize_tables() {
    std::string create_trades_table = R"(
        CREATE TABLE IF NOT EXISTS trades (
            trade_id TEXT PRIMARY KEY NOT NULL,
            symbol TEXT NOT NULL,
            price INTEGER NOT NULL,
            quantity INTEGER NOT NULL,
            buyer_order_id TEXT NOT NULL,
            seller_order_id TEXT NOT NULL,
            buyer_user_id TEXT NOT NULL,
            seller_user_id TEXT NOT NULL,
            timestamp DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    std::string create_symbol_index = R"(
        CREATE INDEX IF NOT EXISTS idx_trades_symbol ON trades(symbol)
    )";
    
    std::string create_timestamp_index = R"(
        CREATE INDEX IF NOT EXISTS idx_trades_timestamp ON trades(timestamp DESC)
    )";
    
    return execute_sql(create_trades_table) &&
           execute_sql(create_symbol_index) &&
           execute_sql(create_timestamp_index);
}

std::string DatabaseManager::timestamp_to_sqlite(const std::chrono::system_clock::time_point& timestamp) const {
    auto time_t_val = std::chrono::system_clock::to_time_t(timestamp);
    auto tm_val = *std::localtime(&time_t_val);
    
    std::stringstream ss;
    ss << std::put_time(&tm_val, "%Y-%m-%d %H:%M:%S");
    
    // 添加毫秒
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()
    ).count() % 1000;
    ss << "." << std::setfill('0') << std::setw(3) << ms;
    
    return ss.str();
}

bool DatabaseManager::execute_sql(const std::string& sql) {
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);
    
    if (rc != SQLITE_OK) {
        std::string error = err_msg;
        sqlite3_free(err_msg);
        return false;
    }
    
    return true;
}

void DatabaseManager::worker() {
    while (running_) {
        std::unique_lock lock(queue_mutex_);
        
        // 等待队列中有数据或停止信号
        queue_cv_.wait(lock, [this]() {
            return !trade_queue_.empty() || !running_;
        });
        
        // 处理队列中的所有成交记录
        std::vector<Trade> trades;
        while (!trade_queue_.empty()) {
            trades.push_back(trade_queue_.front());
            trade_queue_.pop();
        }
        
        lock.unlock();
        
        // 批量插入成交记录
        if (!trades.empty()) {
            std::stringstream ss;
            ss << "INSERT INTO trades (trade_id, symbol, price, quantity, buyer_order_id, seller_order_id, buyer_user_id, seller_user_id, timestamp) VALUES ";
            
            for (size_t i = 0; i < trades.size(); ++i) {
                const auto& trade = trades[i];
                
                if (i > 0) {
                    ss << ",";
                }
                
                ss << "('" << trade.trade_id << "', '" << trade.symbol << "', " << trade.price << ", " << trade.quantity << ", '" << trade.buyer_order_id << "', '" << trade.seller_order_id << "', '" << trade.buyer_user_id << "', '" << trade.seller_user_id << "', '" << timestamp_to_sqlite(trade.timestamp) << "')";
            }
            
            execute_sql(ss.str());
        }
    }
}

void DatabaseManager::add_trade(const Trade& trade) {
    std::unique_lock lock(queue_mutex_);
    trade_queue_.push(trade);
    lock.unlock();
    queue_cv_.notify_one();
}

void DatabaseManager::add_trades(const std::vector<Trade>& trades) {
    if (trades.empty()) {
        return;
    }
    
    std::unique_lock lock(queue_mutex_);
    for (const auto& trade : trades) {
        trade_queue_.push(trade);
    }
    lock.unlock();
    queue_cv_.notify_one();
}



std::vector<Trade> DatabaseManager::get_trades(const std::string& symbol, std::size_t limit) {
    std::vector<Trade> trades;
    
    std::stringstream ss;
    ss << "SELECT trade_id, symbol, price, quantity, buyer_order_id, seller_order_id, buyer_user_id, seller_user_id, timestamp FROM trades WHERE symbol = ? ORDER BY timestamp DESC LIMIT ?";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, ss.str().c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::string error = sqlite3_errmsg(db_);
        return trades;
    }
    
    // 绑定参数
    sqlite3_bind_text(stmt, 1, symbol.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, limit);
    
    // 执行查询
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        Trade trade;
        trade.trade_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)) ? 
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)) : "";
        trade.symbol = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) ? 
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) : "";
        trade.price = sqlite3_column_int64(stmt, 2);
        trade.quantity = sqlite3_column_int64(stmt, 3);
        trade.buyer_order_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) ? 
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) : "";
        trade.seller_order_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)) ? 
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)) : "";
        trade.buyer_user_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) ? 
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
        trade.seller_user_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)) ? 
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)) : "";
        
        // 解析时间戳
        const char* timestamp_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        if (timestamp_str) {
            std::string ts_str(timestamp_str);
            std::tm tm = {};
            std::istringstream ts_ss(ts_str);
            
            if (ts_ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S")) {
                trade.timestamp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
                
                // 检查是否有毫秒
                size_t dot_pos = ts_str.find('.');
                if (dot_pos != std::string::npos) {
                    std::string ms_str = ts_str.substr(dot_pos + 1);
                    if (ms_str.length() >= 3) {
                        ms_str = ms_str.substr(0, 3);
                        int ms = std::stoi(ms_str);
                        trade.timestamp += std::chrono::milliseconds(ms);
                    }
                }
            }
        }
        
        trades.push_back(trade);
    }
    
    sqlite3_finalize(stmt);
    return trades;
}

void DatabaseManager::shutdown() {
    running_ = false;
    queue_cv_.notify_one();
    
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
    
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}