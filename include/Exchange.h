#pragma once

#include <unordered_map>
#include <shared_mutex>
#include <string>
#include "OrderBook.h"
#include "DatabaseManager.h"

class Exchange {
private:
    std::unordered_map<std::string, std::shared_ptr<OrderBook>> order_books_;
    std::shared_mutex order_books_mutex_;
    
    std::unique_ptr<DatabaseManager> db_manager_;
    
    // 获取或创建订单薄
    std::shared_ptr<OrderBook> get_or_create_order_book(const std::string& symbol);

public:
    explicit Exchange(const std::string& db_path = "./exchange.db");
    ~Exchange();
    
    // 提交订单
    std::pair<std::shared_ptr<Order>, std::vector<Trade>> submit_order(
        const std::string& user_id,
        const std::string& symbol,
        Order::Side side,
        Order::Type type,
        Price price,
        Quantity quantity
    );
    
    // 撤单
    bool cancel_order(const std::string& symbol, const std::string& order_id);
    
    // 获取订单信息
    std::optional<std::shared_ptr<Order>> get_order(const std::string& symbol, const std::string& order_id) const;
    
    // 获取市场深度
    OrderBook::MarketDepth get_depth(const std::string& symbol, size_t limit = 10) const;
    
    // 查询成交记录
    std::vector<Trade> get_trades(const std::string& symbol, std::size_t limit = 100) const;
    
    // 获取所有交易对
    std::vector<std::string> get_symbols() const;
    
    // 关闭交易所
    void shutdown();
};
