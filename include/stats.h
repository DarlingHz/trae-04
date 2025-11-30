#ifndef STATS_H
#define STATS_H

#include <string>
#include "database.h"

// 每日统计数据结构
struct DailyStats {
    std::string date;
    int total_orders;
    int paid_orders;
    double paid_amount;
    int shipped_orders;
    double shipped_amount;
    int cancelled_orders;
};

// 统计服务接口
class StatsService {
public:
    StatsService(Database& db);
    
    // 获取指定日期的统计数据
    DailyStats get_daily_stats(const std::string& date);
    
private:
    Database& db_;
};

#endif // STATS_H