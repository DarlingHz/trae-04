#include "OrderBook.h"
#include <random>
#include <sstream>
#include <iomanip>

std::string OrderBook::generate_order_id() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << "ORD";
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    ss << std::hex << ms;
    
    for (int i = 0; i < 8; ++i) {
        ss << std::hex << dis(gen);
    }
    
    return ss.str();
}

std::string OrderBook::generate_trade_id() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << "TRD";
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    ss << std::hex << ms;
    
    for (int i = 0; i < 8; ++i) {
        ss << std::hex << dis(gen);
    }
    
    return ss.str();
}

OrderBook::OrderBook(std::string symbol) : symbol_(std::move(symbol)) {
}

std::vector<Trade> OrderBook::match_buy_order(std::shared_ptr<Order> incoming_order) {
    std::vector<Trade> trades;
    
    auto it = asks_.begin();
    while (it != asks_.end() && incoming_order->quantity > incoming_order->filled_quantity) {
        auto& price_level = it->second;
        
        while (!price_level.empty() && incoming_order->quantity > incoming_order->filled_quantity) {
            auto resting_order = price_level.front();
            
            if (resting_order->is_cancelled) {
                price_level.pop();
                continue;
            }
            
            // 检查价格是否匹配
            bool price_match = incoming_order->type == Order::Type::MARKET || incoming_order->price >= resting_order->price;
            
            if (!price_match) {
                break;
            }
            
            // 计算成交数量
            Quantity trade_quantity = std::min(
                incoming_order->quantity - incoming_order->filled_quantity,
                resting_order->quantity - resting_order->filled_quantity
            );
            
            // 创建成交记录
            Trade trade;
            trade.trade_id = generate_trade_id();
            trade.symbol = symbol_;
            trade.price = resting_order->price;  // 使用 resting order 的价格
            trade.quantity = trade_quantity;
            trade.buyer_order_id = incoming_order->order_id;
            trade.seller_order_id = resting_order->order_id;
            trade.buyer_user_id = incoming_order->user_id;
            trade.seller_user_id = resting_order->user_id;
            trade.timestamp = std::chrono::system_clock::now();
            
            trades.push_back(trade);
            
            // 更新订单成交数量
            incoming_order->filled_quantity += trade_quantity;
            resting_order->filled_quantity += trade_quantity;
            
            // 如果 resting order 完全成交，从队列中移除
            if (resting_order->filled_quantity == resting_order->quantity) {
                price_level.pop();
            }
        }
        
        // 如果当前价格 level 为空，移除该 level
        if (price_level.empty()) {
            it = asks_.erase(it);
        } else {
            ++it;
        }
    }
    
    return trades;
}

std::vector<Trade> OrderBook::match_sell_order(std::shared_ptr<Order> incoming_order) {
    std::vector<Trade> trades;
    
    auto it = bids_.begin();
    while (it != bids_.end() && incoming_order->quantity > incoming_order->filled_quantity) {
        auto& price_level = it->second;
        
        while (!price_level.empty() && incoming_order->quantity > incoming_order->filled_quantity) {
            auto resting_order = price_level.front();
            
            if (resting_order->is_cancelled) {
                price_level.pop();
                continue;
            }
            
            // 检查价格是否匹配
            bool price_match = incoming_order->type == Order::Type::MARKET || incoming_order->price <= resting_order->price;
            
            if (!price_match) {
                break;
            }
            
            // 计算成交数量
            Quantity trade_quantity = std::min(
                incoming_order->quantity - incoming_order->filled_quantity,
                resting_order->quantity - resting_order->filled_quantity
            );
            
            // 创建成交记录
            Trade trade;
            trade.trade_id = generate_trade_id();
            trade.symbol = symbol_;
            trade.price = resting_order->price;  // 使用 resting order 的价格
            trade.quantity = trade_quantity;
            trade.buyer_order_id = resting_order->order_id;
            trade.seller_order_id = incoming_order->order_id;
            trade.buyer_user_id = resting_order->user_id;
            trade.seller_user_id = incoming_order->user_id;
            trade.timestamp = std::chrono::system_clock::now();
            
            trades.push_back(trade);
            
            // 更新订单成交数量
            incoming_order->filled_quantity += trade_quantity;
            resting_order->filled_quantity += trade_quantity;
            
            // 如果 resting order 完全成交，从队列中移除
            if (resting_order->filled_quantity == resting_order->quantity) {
                price_level.pop();
            }
        }
        
        // 如果当前价格 level 为空，移除该 level
        if (price_level.empty()) {
            it = bids_.erase(it);
        } else {
            ++it;
        }
    }
    
    return trades;
}

std::vector<Trade> OrderBook::match_orders(std::shared_ptr<Order> incoming_order) {
    if (incoming_order->side == Order::Side::BUY) {
        return match_buy_order(incoming_order);
    } else {
        return match_sell_order(incoming_order);
    }
}

std::vector<Trade> OrderBook::process_limit_order(std::shared_ptr<Order> order) {
    auto trades = match_orders(order);
    
    // 如果订单未完全成交，加入订单薄
    if (order->filled_quantity < order->quantity && !order->is_cancelled) {
        if (order->side == Order::Side::BUY) {
            bids_[order->price].push(order);
        } else {
            asks_[order->price].push(order);
        }
    }
    
    return trades;
}

std::vector<Trade> OrderBook::process_market_order(std::shared_ptr<Order> order) {
    // 市价单直接撮合，不加入订单薄
    return match_orders(order);
}

std::pair<std::shared_ptr<Order>, std::vector<Trade>> OrderBook::submit_order(
    const std::string& user_id,
    Order::Side side,
    Order::Type type,
    Price price,
    Quantity quantity
) {
    std::unique_lock lock(mutex_);
    
    // 创建订单
    auto order = std::make_shared<Order>();
    order->order_id = generate_order_id();
    order->user_id = user_id;
    order->symbol = symbol_;
    order->side = side;
    order->type = type;
    order->price = price;
    order->quantity = quantity;
    order->timestamp = std::chrono::system_clock::now();
    
    // 存储订单索引
    orders_[order->order_id] = order;
    
    // 处理订单
    std::vector<Trade> trades;
    if (type == Order::Type::LIMIT) {
        trades = process_limit_order(order);
    } else {
        trades = process_market_order(order);
    }
    
    return {order, trades};
}

bool OrderBook::cancel_order(const std::string& order_id) {
    std::unique_lock lock(mutex_);
    
    auto order_it = orders_.find(order_id);
    if (order_it == orders_.end()) {
        return false;
    }
    
    auto order = order_it->second;
    if (order->is_cancelled || order->filled_quantity == order->quantity) {
        return false;
    }
    
    order->is_cancelled = true;
    return true;
}

std::optional<std::shared_ptr<Order>> OrderBook::get_order(const std::string& order_id) const {
    std::shared_lock lock(mutex_);
    
    auto it = orders_.find(order_id);
    if (it == orders_.end()) {
        return std::nullopt;
    }
    
    return it->second;
}

OrderBook::MarketDepth OrderBook::get_depth(size_t limit) const {
    std::shared_lock lock(mutex_);
    
    MarketDepth depth;
    depth.bids.reserve(limit);
    depth.asks.reserve(limit);
    
    // 获取买单深度
    size_t count = 0;
    for (const auto& [price, queue] : bids_) {
        if (count >= limit) {
            break;
        }
        
        // 计算该价格 level 的总数量（排除已撤销和已完全成交的订单）
        Quantity total_quantity = 0;
        auto temp_queue = queue;
        while (!temp_queue.empty()) {
            auto order = temp_queue.front();
            temp_queue.pop();
            if (!order->is_cancelled && order->filled_quantity < order->quantity) {
                total_quantity += (order->quantity - order->filled_quantity);
            }
        }
        
        if (total_quantity > 0) {
            depth.bids.push_back({price, total_quantity});
            count++;
        }
    }
    
    // 获取卖单深度
    count = 0;
    for (const auto& [price, queue] : asks_) {
        if (count >= limit) {
            break;
        }
        
        // 计算该价格 level 的总数量（排除已撤销和已完全成交的订单）
        Quantity total_quantity = 0;
        auto temp_queue = queue;
        while (!temp_queue.empty()) {
            auto order = temp_queue.front();
            temp_queue.pop();
            if (!order->is_cancelled && order->filled_quantity < order->quantity) {
                total_quantity += (order->quantity - order->filled_quantity);
            }
        }
        
        if (total_quantity > 0) {
            depth.asks.push_back({price, total_quantity});
            count++;
        }
    }
    
    return depth;
}

size_t OrderBook::get_bid_count() const {
    std::shared_lock lock(mutex_);
    return bids_.size();
}

size_t OrderBook::get_ask_count() const {
    std::shared_lock lock(mutex_);
    return asks_.size();
}

size_t OrderBook::get_total_order_count() const {
    std::shared_lock lock(mutex_);
    return orders_.size();
}
