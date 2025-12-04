#include "Exchange.h"

Exchange::Exchange(const std::string& db_path) {
    try {
        db_manager_ = std::make_unique<DatabaseManager>(db_path);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to initialize database manager: " + std::string(e.what()));
    }
}

Exchange::~Exchange() {
    shutdown();
}

std::shared_ptr<OrderBook> Exchange::get_or_create_order_book(const std::string& symbol) {
    std::unique_lock lock(order_books_mutex_);
    
    auto it = order_books_.find(symbol);
    if (it != order_books_.end()) {
        return it->second;
    }
    
    // 创建新的订单薄
    auto order_book = std::make_shared<OrderBook>(symbol);
    order_books_[symbol] = order_book;
    
    return order_book;
}

std::pair<std::shared_ptr<Order>, std::vector<Trade>> Exchange::submit_order(
    const std::string& user_id,
    const std::string& symbol,
    Order::Side side,
    Order::Type type,
    Price price,
    Quantity quantity
) {
    auto order_book = get_or_create_order_book(symbol);
    auto [order, trades] = order_book->submit_order(user_id, side, type, price, quantity);
    
    // 将成交记录异步写入数据库
    if (!trades.empty() && db_manager_) {
        db_manager_->add_trades(trades);
    }
    
    return {order, trades};
}

bool Exchange::cancel_order(const std::string& symbol, const std::string& order_id) {
    std::shared_lock lock(order_books_mutex_);
    
    auto it = order_books_.find(symbol);
    if (it == order_books_.end()) {
        return false;
    }
    
    return it->second->cancel_order(order_id);
}

std::optional<std::shared_ptr<Order>> Exchange::get_order(const std::string& symbol, const std::string& order_id) const {
    std::shared_lock<std::shared_mutex> lock(const_cast<std::shared_mutex&>(order_books_mutex_));
    
    auto it = order_books_.find(symbol);
    if (it == order_books_.end()) {
        return std::nullopt;
    }
    
    return it->second->get_order(order_id);
}

OrderBook::MarketDepth Exchange::get_depth(const std::string& symbol, size_t limit) const {
    std::shared_lock<std::shared_mutex> lock(const_cast<std::shared_mutex&>(order_books_mutex_));
    
    auto it = order_books_.find(symbol);
    if (it == order_books_.end()) {
        return {};
    }
    
    return it->second->get_depth(limit);
}

std::vector<Trade> Exchange::get_trades(const std::string& symbol, std::size_t limit) const {
    if (!db_manager_) {
        return {};
    }
    
    return db_manager_->get_trades(symbol, limit);
}

std::vector<std::string> Exchange::get_symbols() const {
    std::shared_lock<std::shared_mutex> lock(const_cast<std::shared_mutex&>(order_books_mutex_));
    
    std::vector<std::string> symbols;
    symbols.reserve(order_books_.size());
    
    for (const auto& [symbol, _] : order_books_) {
        symbols.push_back(symbol);
    }
    
    return symbols;
}

void Exchange::shutdown() {
    if (db_manager_) {
        db_manager_->shutdown();
        db_manager_.reset();
    }
    
    std::unique_lock lock(order_books_mutex_);
    order_books_.clear();
}
