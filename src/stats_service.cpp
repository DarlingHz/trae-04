#include "stats.h"
#include "database.h"
#include <iostream>
#include <sstream>

StatsService::StatsService(Database& db) : db_(db) {
}

DailyStats StatsService::get_daily_stats(const std::string& date) {
    DailyStats stats;
    stats.date = date;
    
    // 构建 SQL 语句，获取指定日期的所有统计数据
    std::stringstream sql;
    sql << "SELECT "
        << "COUNT(*) AS total_orders, "
        << "SUM(CASE WHEN status = 'PAID' THEN 1 ELSE 0 END) AS paid_orders, "
        << "SUM(CASE WHEN status = 'PAID' THEN total_amount ELSE 0 END) AS paid_amount, "
        << "SUM(CASE WHEN status = 'SHIPPED' THEN 1 ELSE 0 END) AS shipped_orders, "
        << "SUM(CASE WHEN status = 'SHIPPED' THEN total_amount ELSE 0 END) AS shipped_amount, "
        << "SUM(CASE WHEN status = 'CANCELLED' THEN 1 ELSE 0 END) AS cancelled_orders "
        << "FROM orders WHERE DATE(created_at) = '" << date << "';";
    
    // 执行查询
    auto result = db_.execute_query(sql.str());
    
    // 填充统计数据
    if (!result.empty()) {
        const auto& row = result[0];
        
        auto total_orders_it = row.find("total_orders");
        if (total_orders_it != row.end()) {
            stats.total_orders = std::stoi(total_orders_it->second);
        }
        auto paid_orders_it = row.find("paid_orders");
        if (paid_orders_it != row.end()) {
            stats.paid_orders = std::stoi(paid_orders_it->second);
        }
        auto paid_amount_it = row.find("paid_amount");
        if (paid_amount_it != row.end()) {
            stats.paid_amount = std::stod(paid_amount_it->second);
        }
        auto shipped_orders_it = row.find("shipped_orders");
        if (shipped_orders_it != row.end()) {
            stats.shipped_orders = std::stoi(shipped_orders_it->second);
        }
        auto shipped_amount_it = row.find("shipped_amount");
        if (shipped_amount_it != row.end()) {
            stats.shipped_amount = std::stod(shipped_amount_it->second);
        }
        auto cancelled_orders_it = row.find("cancelled_orders");
        if (cancelled_orders_it != row.end()) {
            stats.cancelled_orders = std::stoi(cancelled_orders_it->second);
        }
    }
    
    return stats;
}