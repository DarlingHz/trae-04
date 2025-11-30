#ifndef ORDER_H
#define ORDER_H

#include <string>
#include <vector>
#include "database.h"

// 订单状态枚举
enum class OrderStatus {
    PENDING,
    PAID,
    CANCELLED,
    SHIPPED
};

// 订单明细数据结构
struct OrderItem {
    int id;
    int order_id;
    int product_id;
    int quantity;
    double unit_price;
    double subtotal;
};

// 订单数据结构
struct Order {
    int id;
    OrderStatus status;
    double total_amount;
    std::string created_at;
    std::string updated_at;
    std::vector<OrderItem> items;
};

// 订单创建请求中的商品项
struct OrderCreateItem {
    int product_id;
    int quantity;
};

// 订单创建请求
struct OrderCreateRequest {
    std::vector<OrderCreateItem> items;
};

// 订单状态更新请求
struct OrderStatusUpdateRequest {
    OrderStatus status;
    bool restock = false; // 仅当状态变为 CANCELLED 时有效
};

// 订单服务接口
class OrderService {
public:
    OrderService(Database& db);
    
    // 创建订单
    Order create_order(const std::vector<OrderCreateItem>& items);
    
    // 获取单个订单
    Order get_order(int id);
    
    // 获取订单列表（分页）
    std::vector<Order> get_orders(int page, int page_size, OrderStatus status = OrderStatus::PENDING, 
                                     const std::string& start_date = "", const std::string& end_date = "");
    
    // 更新订单状态
    bool update_order_status(int order_id, OrderStatus new_status, bool restock = false);
    
    // 获取订单总数
    int get_order_count(OrderStatus status = OrderStatus::PENDING, 
                         const std::string& start_date = "", const std::string& end_date = "");
    
private:
    Database& db_;
    
    // 将字符串转换为 OrderStatus 枚举
    OrderStatus string_to_order_status(const std::string& status_str);
    
public:
    // 将 OrderStatus 枚举转换为字符串
    std::string order_status_to_string(OrderStatus status) const;
};

#endif // ORDER_H