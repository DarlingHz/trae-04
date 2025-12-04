#pragma once

#include <crow.h>
#include "Exchange.h"

class ApiHandler {
private:
    std::shared_ptr<Exchange> exchange_;
    
    // 辅助函数：解析订单方向
    std::optional<Order::Side> parse_side(const std::string& side_str) const;
    
    // 辅助函数：解析订单类型
    std::optional<Order::Type> parse_type(const std::string& type_str) const;
    
    // 辅助函数：将价格从浮点数转换为定点数
    Price parse_price(double price) const;
    
    // 辅助函数：将数量从浮点数转换为定点数
    Quantity parse_quantity(double quantity) const;
    
    // 辅助函数：将定点数转换为浮点数
    double to_double(int64_t value) const;
    
    // API处理函数
    void handle_submit_order(const crow::request& req, crow::response& res);
    void handle_cancel_order(const crow::request& req, crow::response& res);
    void handle_cancel_order(const std::string& symbol, const std::string& order_id, crow::response& res);
    void handle_get_depth(const crow::request& req, crow::response& res);
    void handle_get_trades(const crow::request& req, crow::response& res);
    void handle_health_check(const crow::request& req, crow::response& res);

public:
    explicit ApiHandler(std::shared_ptr<Exchange> exchange);
    
    // 注册API路由
    void register_routes(crow::SimpleApp& app);
};
