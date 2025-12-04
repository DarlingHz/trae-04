#include "ApiHandler.h"
#include <crow/json.h>
#include <stdexcept>

ApiHandler::ApiHandler(std::shared_ptr<Exchange> exchange) : exchange_(std::move(exchange)) {
}

std::optional<Order::Side> ApiHandler::parse_side(const std::string& side_str) const {
    if (side_str == "buy" || side_str == "BUY") {
        return Order::Side::BUY;
    } else if (side_str == "sell" || side_str == "SELL") {
        return Order::Side::SELL;
    }
    return std::nullopt;
}

std::optional<Order::Type> ApiHandler::parse_type(const std::string& type_str) const {
    if (type_str == "limit" || type_str == "LIMIT") {
        return Order::Type::LIMIT;
    } else if (type_str == "market" || type_str == "MARKET") {
        return Order::Type::MARKET;
    }
    return std::nullopt;
}

Price ApiHandler::parse_price(double price) const {
    // 价格精度：1e8
    return static_cast<Price>(price * 100000000.0);
}

Quantity ApiHandler::parse_quantity(double quantity) const {
    // 数量精度：1e8
    return static_cast<Quantity>(quantity * 100000000.0);
}

double ApiHandler::to_double(int64_t value) const {
    return static_cast<double>(value) / 100000000.0;
}

void ApiHandler::handle_submit_order(const crow::request& req, crow::response& res) {
    try {
        auto json = crow::json::load(req.body);
        if (!json) {
            res.code = 400;
            crow::json::wvalue response; response["error"] = "Invalid JSON"; res.write(response.dump());
            res.end();
            return;
        }
        
        // 验证必填字段
        if (!json.has("user_id") || !json.has("symbol") || !json.has("side") || !json.has("type") || !json.has("quantity")) {
            res.code = 400;
            crow::json::wvalue response; response["error"] = "Missing required fields"; res.write(response.dump());
            res.end();
            return;
        }
        
        // 解析字段
        std::string user_id = json["user_id"].s();
        std::string symbol = json["symbol"].s();
        auto side = parse_side(json["side"].s());
        auto type = parse_type(json["type"].s());
        double quantity_double = json["quantity"].d();
        
        if (!side) {
            res.code = 400;
            crow::json::wvalue response; response["error"] = "Invalid side (must be buy or sell)"; res.write(response.dump());
            res.end();
            return;
        }
        
        if (!type) {
            res.code = 400;
            crow::json::wvalue response; response["error"] = "Invalid type (must be limit or market)"; res.write(response.dump());
            res.end();
            return;
        }
        
        if (quantity_double <= 0) {
            res.code = 400;
            crow::json::wvalue response; response["error"] = "Quantity must be positive"; res.write(response.dump());
            res.end();
            return;
        }
        
        // 解析价格（仅对限价单）
        Price price = 0;
        if (*type == Order::Type::LIMIT) {
            if (!json.has("price")) {
                res.code = 400;
                crow::json::wvalue response; response["error"] = "Price is required for limit orders"; res.write(response.dump());
                res.end();
                return;
            }
            
            double price_double = json["price"].d();
            if (price_double <= 0) {
                res.code = 400;
                crow::json::wvalue response; response["error"] = "Price must be positive"; res.write(response.dump());
                res.end();
                return;
            }
            
            price = parse_price(price_double);
        }
        
        Quantity quantity = parse_quantity(quantity_double);
        
        // 提交订单
        auto [order, trades] = exchange_->submit_order(user_id, symbol, *side, *type, price, quantity);
        
        // 构建响应
        crow::json::wvalue response;
        response["success"] = true;
        response["order_id"] = order->order_id;
        response["symbol"] = order->symbol;
        response["side"] = order->side == Order::Side::BUY ? "buy" : "sell";
        response["type"] = order->type == Order::Type::LIMIT ? "limit" : "market";
        if (order->type == Order::Type::LIMIT) { response["price"] = to_double(order->price); }
        response["quantity"] = to_double(order->quantity);
        response["filled_quantity"] = to_double(order->filled_quantity);
        response["status"] = order->is_cancelled ? "cancelled" : 
                             (order->filled_quantity == order->quantity ? "filled" : 
                             (order->filled_quantity > 0 ? "partially_filled" : "pending"));
        
        // 添加成交记录
        if (!trades.empty()) {
            crow::json::wvalue::list trades_json;
            for (const auto& trade : trades) {
                crow::json::wvalue trade_json;
                trade_json["trade_id"] = trade.trade_id;
                trade_json["price"] = to_double(trade.price);
                trade_json["quantity"] = to_double(trade.quantity);
                trade_json["buyer_order_id"] = trade.buyer_order_id;
                trade_json["seller_order_id"] = trade.seller_order_id;
                trades_json.push_back(std::move(trade_json));
            }
            response["trades"] = std::move(trades_json);
        }
        
        res.write(response.dump());
        res.end();
        
    } catch (const std::exception& e) {
        res.code = 500;
        crow::json::wvalue response; response["error"] = e.what(); res.write(response.dump());
        res.end();
    }
}

void ApiHandler::handle_cancel_order(const crow::request& req, crow::response& res) {
    try {
        auto symbol = req.url_params.get("symbol");
        auto order_id = req.url_params.get("order_id");
        
        if (!symbol || !order_id) {
            res.code = 400;
            crow::json::wvalue response; response["error"] = "Missing symbol or order_id"; res.write(response.dump());
            res.end();
            return;
        }
        
        handle_cancel_order(symbol, order_id, res);
        
    } catch (const std::exception& e) {
        res.code = 500;
        crow::json::wvalue response; response["error"] = e.what(); res.write(response.dump());
        res.end();
    }
}

void ApiHandler::handle_cancel_order(const std::string& symbol, const std::string& order_id, crow::response& res) {
    try {
        bool success = exchange_->cancel_order(symbol, order_id);
        
        crow::json::wvalue response;
        response["success"] = success;
        
        if (!success) {
            response["error"] = "Order not found or already cancelled/filled";
        }
        
        res.write(response.dump());
        res.end();
        
    } catch (const std::exception& e) {
        res.code = 500;
        crow::json::wvalue response; response["error"] = e.what(); res.write(response.dump());
        res.end();
    }
}

void ApiHandler::handle_get_depth(const crow::request& req, crow::response& res) {
    try {
        auto symbol = req.url_params.get("symbol");
        auto limit_str = req.url_params.get("limit");
        
        if (!symbol) {
            res.code = 400;
            crow::json::wvalue response; response["error"] = "Missing symbol"; res.write(response.dump());
            res.end();
            return;
        }
        
        size_t limit = 10;
        if (limit_str) {
            limit = std::stoul(limit_str);
            if (limit > 100) {  // 限制最大深度
                limit = 100;
            }
        }
        
        auto depth = exchange_->get_depth(symbol, limit);
        
        // 构建响应
        crow::json::wvalue response;
        response["symbol"] = symbol;
        
        crow::json::wvalue::list bids_json;
        for (const auto& level : depth.bids) {
            crow::json::wvalue level_json;
            level_json["price"] = to_double(level.price);
            level_json["quantity"] = to_double(level.quantity);
            bids_json.push_back(std::move(level_json));
        }
        response["bids"] = std::move(bids_json);
        
        crow::json::wvalue::list asks_json;
        for (const auto& level : depth.asks) {
            crow::json::wvalue level_json;
            level_json["price"] = to_double(level.price);
            level_json["quantity"] = to_double(level.quantity);
            asks_json.push_back(std::move(level_json));
        }
        response["asks"] = std::move(asks_json);
        
        res.write(response.dump());
        res.end();
        
    } catch (const std::exception& e) {
        res.code = 500;
        crow::json::wvalue response; response["error"] = e.what(); res.write(response.dump());
        res.end();
    }
}

void ApiHandler::handle_get_trades(const crow::request& req, crow::response& res) {
    try {
        auto symbol = req.url_params.get("symbol");
        
        if (!symbol) {
            res.code = 400;
            crow::json::wvalue response; response["error"] = "Missing symbol"; res.write(response.dump());
            res.end();
            return;
        }
        
        auto trades = exchange_->get_trades(symbol);
        
        // 构建响应
        crow::json::wvalue::list trades_json;
        for (const auto& trade : trades) {
            crow::json::wvalue trade_json;
            trade_json["trade_id"] = trade.trade_id;
            trade_json["symbol"] = trade.symbol;
            trade_json["price"] = to_double(trade.price);
            trade_json["quantity"] = to_double(trade.quantity);
            trade_json["buyer_order_id"] = trade.buyer_order_id;
            trade_json["seller_order_id"] = trade.seller_order_id;
            trade_json["buyer_user_id"] = trade.buyer_user_id;
            trade_json["seller_user_id"] = trade.seller_user_id;
            
            // 格式化时间戳
            auto time_t_val = std::chrono::system_clock::to_time_t(trade.timestamp);
            auto tm_val = *std::localtime(&time_t_val);
            
            std::stringstream ss;
            ss << std::put_time(&tm_val, "%Y-%m-%d %H:%M:%S");
            
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                trade.timestamp.time_since_epoch()
            ).count() % 1000;
            ss << "." << std::setfill('0') << std::setw(3) << ms;
            
            trade_json["timestamp"] = ss.str();
            trades_json.push_back(std::move(trade_json));
        }
        
        res.write(crow::json::wvalue(trades_json).dump());
        res.end();
        
    } catch (const std::exception& e) {
        res.code = 500;
        crow::json::wvalue response; response["error"] = e.what(); res.write(response.dump());
        res.end();
    }
}

void ApiHandler::handle_health_check(const crow::request& /*req*/, crow::response& res) {
    crow::json::wvalue response;
    response["status"] = "ok";
    response["message"] = "Exchange API is running";
    res.write(response.dump());
    res.end();
}

void ApiHandler::register_routes(crow::SimpleApp& app) {
    // 健康检查
    CROW_ROUTE(app, ("/api/health"))
        .methods("GET"_method)([this](const crow::request& req, crow::response& res) {
            handle_health_check(req, res);
        });
    
    // 提交订单
    CROW_ROUTE(app, ("/api/order"))
        .methods("POST"_method)([this](const crow::request& req, crow::response& res) {
            handle_submit_order(req, res);
        });
    
    // 撤单
    CROW_ROUTE(app, ("/api/order/<string>/<string>"))
        .methods("DELETE"_method)([this](const crow::request& /*req*/, crow::response& res, std::string symbol, std::string order_id) {
            // 直接调用handle_cancel_order，传递symbol和order_id
        handle_cancel_order(symbol, order_id, res);
        });
    
    // 获取市场深度
    CROW_ROUTE(app, ("/api/depth"))
        .methods("GET"_method)([this](const crow::request& req, crow::response& res) {
            handle_get_depth(req, res);
        });
    
    // 查询成交记录
    CROW_ROUTE(app, ("/api/trades"))
        .methods("GET"_method)([this](const crow::request& req, crow::response& res) {
            handle_get_trades(req, res);
        });
}
