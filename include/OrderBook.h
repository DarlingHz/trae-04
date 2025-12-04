#pragma once

#include <map>
#include <queue>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>
#include <optional>
#include <atomic>

// 使用定点数表示价格和数量，避免浮点数精度问题
// 价格：1e8精度（例如，0.01美元表示为1000000）
// 数量：1e8精度（例如，0.0001 BTC表示为10000）
typedef int64_t Price;
typedef int64_t Quantity;

struct Order {
    enum class Side { BUY, SELL };
    enum class Type { LIMIT, MARKET };

    std::string order_id;
    std::string user_id;
    std::string symbol;
    Side side;
    Type type;
    Price price;      // 仅对限价单有效
    Quantity quantity;
    Quantity filled_quantity = 0;
    std::chrono::system_clock::time_point timestamp;
    bool is_cancelled = false;
};

struct Trade {
    std::string trade_id;
    std::string symbol;
    Price price;
    Quantity quantity;
    std::string buyer_order_id;
    std::string seller_order_id;
    std::string buyer_user_id;
    std::string seller_user_id;
    std::chrono::system_clock::time_point timestamp;
};

// 订单队列：按时间优先排序
using OrderQueue = std::queue<std::shared_ptr<Order>>;

class OrderBook {
private:
        std::string symbol_;
        
        // 买单：价格从高到低排序
        std::map<Price, OrderQueue, std::greater<Price>> bids_;
        
        // 卖单：价格从低到高排序
        std::map<Price, OrderQueue, std::less<Price>> asks_;
        
        // 所有订单的索引，用于快速查找
        std::unordered_map<std::string, std::shared_ptr<Order>> orders_;
        
        // 读写锁，支持多读者单写者
        mutable std::shared_mutex mutex_;
        
        // 生成唯一订单ID
        std::string generate_order_id() const;
        
        // 生成唯一成交ID
        std::string generate_trade_id() const;
        
        // 撮合买单
        std::vector<Trade> match_buy_order(std::shared_ptr<Order> incoming_order);
        
        // 撮合卖单
        std::vector<Trade> match_sell_order(std::shared_ptr<Order> incoming_order);
        
        // 撮合买单和卖单
        std::vector<Trade> match_orders(std::shared_ptr<Order> incoming_order);
        
        // 处理限价单
        std::vector<Trade> process_limit_order(std::shared_ptr<Order> order);
        
        // 处理市价单
        std::vector<Trade> process_market_order(std::shared_ptr<Order> order);

public:
    explicit OrderBook(std::string symbol);
    
    // 提交订单
    std::pair<std::shared_ptr<Order>, std::vector<Trade>> submit_order(
        const std::string& user_id,
        Order::Side side,
        Order::Type type,
        Price price,
        Quantity quantity
    );
    
    // 撤单
    bool cancel_order(const std::string& order_id);
    
    // 获取订单信息
    std::optional<std::shared_ptr<Order>> get_order(const std::string& order_id) const;
    
    // 获取市场深度
    struct DepthLevel {
        Price price;
        Quantity quantity;
    };
    
    struct MarketDepth {
        std::vector<DepthLevel> bids;
        std::vector<DepthLevel> asks;
    };
    
    MarketDepth get_depth(size_t limit = 10) const;
    
    // 获取订单薄状态
    size_t get_bid_count() const;
    size_t get_ask_count() const;
    size_t get_total_order_count() const;
};
