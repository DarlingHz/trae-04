#include "order.h"
#include "database.h"
#include "product.h"
#include <iostream>
#include <sstream>
#include <algorithm>

OrderService::OrderService(Database& db) : db_(db) {
}

Order OrderService::create_order(const std::vector<OrderCreateItem>& items) {
    Order order;
    
    // 开始事务
    if (!db_.begin_transaction()) {
        return order;
    }
    
    try {
        // 批量获取所有商品信息，避免多次查询
        std::vector<int> product_ids;
        for (const auto& item : items) {
            product_ids.push_back(item.product_id);
        }
        
        // 构建商品查询 SQL
        std::stringstream product_sql;
        product_sql << "SELECT * FROM products WHERE id IN (";
        for (size_t i = 0; i < product_ids.size(); ++i) {
            if (i > 0) {
                product_sql << ",";
            }
            product_sql << product_ids[i];
        }
        product_sql << ");";
        
        auto product_result = db_.execute_query(product_sql.str());
        
        // 将商品结果转换为 map，便于查找
        std::map<int, Product> product_map;
        for (const auto& row : product_result) {
            Product product;
            auto id_it = row.find("id");
            if (id_it != row.end()) {
                product.id = std::stoi(id_it->second);
            }
            auto name_it = row.find("name");
            if (name_it != row.end()) {
                product.name = name_it->second;
            }
            auto sku_it = row.find("sku");
            if (sku_it != row.end()) {
                product.sku = sku_it->second;
            }
            auto price_it = row.find("price");
            if (price_it != row.end()) {
                product.price = std::stod(price_it->second);
            }
            auto stock_it = row.find("stock");
            if (stock_it != row.end()) {
                product.stock = std::stoi(stock_it->second);
            }
            auto reorder_threshold_it = row.find("reorder_threshold");
            if (reorder_threshold_it != row.end()) {
                product.reorder_threshold = std::stoi(reorder_threshold_it->second);
            }
            auto created_at_it = row.find("created_at");
            if (created_at_it != row.end()) {
                product.created_at = created_at_it->second;
            }
            auto updated_at_it = row.find("updated_at");
            if (updated_at_it != row.end()) {
                product.updated_at = updated_at_it->second;
            }
            
            product_map[product.id] = product;
        }
        
        // 检查每个商品的库存是否足够
        double total_amount = 0.0;
        std::vector<OrderItem> order_items;
        
        for (const auto& item : items) {
            auto it = product_map.find(item.product_id);
            if (it == product_map.end()) {
                // 商品不存在
                db_.rollback_transaction();
                return order;
            }
            
            const Product& product = it->second;
            if (product.stock < item.quantity) {
                // 库存不足
                db_.rollback_transaction();
                return order;
            }
            
            // 计算商品小计
            double subtotal = product.price * item.quantity;
            total_amount += subtotal;
            
            // 创建订单明细
            OrderItem order_item;
            order_item.product_id = product.id;
            order_item.quantity = item.quantity;
            order_item.unit_price = product.price;
            order_item.subtotal = subtotal;
            
            order_items.push_back(order_item);
        }
        
        // 创建订单
        std::stringstream order_sql;
        order_sql << "INSERT INTO orders (status, total_amount) VALUES ('PENDING', " << total_amount << ");";
        
        if (!db_.execute_non_query(order_sql.str())) {
            db_.rollback_transaction();
            return order;
        }
        
        int order_id = static_cast<int>(db_.get_last_insert_rowid());
        
        // 插入订单明细并更新商品库存
        for (auto& item : order_items) {
            // 插入订单明细
            std::stringstream item_sql;
            item_sql << "INSERT INTO order_items (order_id, product_id, quantity, unit_price, subtotal) "
                     << "VALUES (" << order_id << ", " << item.product_id << ", " << item.quantity << ", " << item.unit_price << ", " << item.subtotal << ");";
            
            if (!db_.execute_non_query(item_sql.str())) {
                db_.rollback_transaction();
                return order;
            }
            
            // 更新商品库存
            Product product = product_map[item.product_id];
            int new_stock = product.stock - item.quantity;
            
            std::stringstream update_stock_sql;
            update_stock_sql << "UPDATE products SET stock = " << new_stock << " WHERE id = " << item.product_id << ";";
            
            if (!db_.execute_non_query(update_stock_sql.str())) {
                db_.rollback_transaction();
                return order;
            }
            
            // 记录库存变动日志
            std::stringstream log_sql;
            log_sql << "INSERT INTO stock_logs (product_id, delta, reason, previous_stock, new_stock) "
                    << "VALUES (" << item.product_id << ", -" << item.quantity << ", 'order_created', " << product.stock << ", " << new_stock << ");";
            
            db_.execute_non_query(log_sql.str());
        }
        
        // 提交事务
        if (!db_.commit_transaction()) {
            return order;
        }
        
        // 获取完整的订单信息
        order = get_order(order_id);
        
        return order;
    } catch (const std::exception& e) {
        std::cerr << "创建订单失败: " << e.what() << std::endl;
        db_.rollback_transaction();
        return order;
    }
}

Order OrderService::get_order(int id) {
    Order order;
    order.id = -1; // 初始化为无效 ID
    
    // 构建订单查询 SQL
    std::stringstream order_sql;
    order_sql << "SELECT * FROM orders WHERE id = " << id << ";";
    
    auto order_result = db_.execute_query(order_sql.str());
    
    if (order_result.empty()) {
        return order;
    }
    
    // 填充订单基本信息
    const auto& order_row = order_result[0];
    auto id_it = order_row.find("id");
    if (id_it != order_row.end()) {
        order.id = std::stoi(id_it->second);
    }
    auto status_it = order_row.find("status");
    if (status_it != order_row.end()) {
        order.status = string_to_order_status(status_it->second);
    }
    auto total_amount_it = order_row.find("total_amount");
    if (total_amount_it != order_row.end()) {
        order.total_amount = std::stod(total_amount_it->second);
    }
    auto created_at_it = order_row.find("created_at");
    if (created_at_it != order_row.end()) {
        order.created_at = created_at_it->second;
    }
    auto updated_at_it = order_row.find("updated_at");
    if (updated_at_it != order_row.end()) {
        order.updated_at = updated_at_it->second;
    }
    
    // 构建订单明细查询 SQL
    std::stringstream items_sql;
    items_sql << "SELECT * FROM order_items WHERE order_id = " << id << ";";
    
    auto items_result = db_.execute_query(items_sql.str());
    
    // 填充订单明细
    for (const auto& item_row : items_result) {
        OrderItem item;
        auto item_id_it = item_row.find("id");
        if (item_id_it != item_row.end()) {
            item.id = std::stoi(item_id_it->second);
        }
        auto item_order_id_it = item_row.find("order_id");
        if (item_order_id_it != item_row.end()) {
            item.order_id = std::stoi(item_order_id_it->second);
        }
        auto item_product_id_it = item_row.find("product_id");
        if (item_product_id_it != item_row.end()) {
            item.product_id = std::stoi(item_product_id_it->second);
        }
        auto item_quantity_it = item_row.find("quantity");
        if (item_quantity_it != item_row.end()) {
            item.quantity = std::stoi(item_quantity_it->second);
        }
        auto item_unit_price_it = item_row.find("unit_price");
        if (item_unit_price_it != item_row.end()) {
            item.unit_price = std::stod(item_unit_price_it->second);
        }
        auto item_subtotal_it = item_row.find("subtotal");
        if (item_subtotal_it != item_row.end()) {
            item.subtotal = std::stod(item_subtotal_it->second);
        }
        
        order.items.push_back(item);
    }
    
    return order;
}

std::vector<Order> OrderService::get_orders(int page, int page_size, OrderStatus status, const std::string& start_date, const std::string& end_date) {
    std::vector<Order> orders;
    
    // 计算偏移量
    int offset = (page - 1) * page_size;
    
    // 构建 SQL 语句
    std::stringstream sql;
    sql << "SELECT * FROM orders WHERE 1=1 ";
    
    // 添加状态过滤
    sql << "AND status = '" << order_status_to_string(status) << "' ";
    
    // 添加日期过滤
    if (!start_date.empty()) {
        sql << "AND DATE(created_at) >= '" << start_date << "' ";
    }
    
    if (!end_date.empty()) {
        sql << "AND DATE(created_at) <= '" << end_date << "' ";
    }
    
    // 添加排序和分页
    sql << "ORDER BY created_at DESC LIMIT " << page_size << " OFFSET " << offset << ";";
    
    // 执行查询
    auto result = db_.execute_query(sql.str());
    
    // 转换结果为 Order 对象
    for (const auto& row : result) {
        auto id_it = row.find("id");
        if (id_it != row.end()) {
            Order order = get_order(std::stoi(id_it->second));
            orders.push_back(order);
        }
    }
    
    return orders;
}

bool OrderService::update_order_status(int order_id, OrderStatus new_status, bool restock) {
    // 开始事务
    if (!db_.begin_transaction()) {
        return false;
    }
    
    try {
        // 获取当前订单信息
        Order order = get_order(order_id);
        if (order.id == -1) {
            db_.rollback_transaction();
            return false;
        }
        
        // 检查状态流转是否合理
        if (new_status == order.status) {
            // 状态未变化，幂等操作，直接返回成功
            db_.rollback_transaction();
            return true;
        }
        
        // 合理的状态流转：
        // PENDING -> PAID -> SHIPPED
        // PENDING -> CANCELLED
        // PAID -> CANCELLED
        bool is_valid_transition = false;
        
        if (order.status == OrderStatus::PENDING) {
            if (new_status == OrderStatus::PAID || new_status == OrderStatus::CANCELLED) {
                is_valid_transition = true;
            }
        } else if (order.status == OrderStatus::PAID) {
            if (new_status == OrderStatus::SHIPPED || new_status == OrderStatus::CANCELLED) {
                is_valid_transition = true;
            }
        } else if (order.status == OrderStatus::SHIPPED) {
            // 已发货订单不能再修改状态
            is_valid_transition = false;
        } else if (order.status == OrderStatus::CANCELLED) {
            // 已取消订单不能再修改状态
            is_valid_transition = false;
        }
        
        if (!is_valid_transition) {
            db_.rollback_transaction();
            return false;
        }
        
        // 如果是取消订单且需要回滚库存
        if (new_status == OrderStatus::CANCELLED && restock) {
            for (const auto& item : order.items) {
                // 获取商品当前库存
                std::stringstream product_sql;
                product_sql << "SELECT stock FROM products WHERE id = " << item.product_id << ";";
                
                auto product_result = db_.execute_query(product_sql.str());
                if (product_result.empty()) {
                    db_.rollback_transaction();
                    return false;
                }
                
                int current_stock = std::stoi(product_result[0]["stock"]);
                int new_stock = current_stock + item.quantity;
                
                // 更新商品库存
                std::stringstream update_stock_sql;
                update_stock_sql << "UPDATE products SET stock = " << new_stock << " WHERE id = " << item.product_id << ";";
                
                if (!db_.execute_non_query(update_stock_sql.str())) {
                    db_.rollback_transaction();
                    return false;
                }
                
                // 记录库存变动日志
                std::stringstream log_sql;
                log_sql << "INSERT INTO stock_logs (product_id, delta, reason, previous_stock, new_stock) "
                        << "VALUES (" << item.product_id << ", " << item.quantity << ", 'order_cancelled', " << current_stock << ", " << new_stock << ");";
                
                db_.execute_non_query(log_sql.str());
            }
        }
        
        // 更新订单状态
        std::stringstream update_sql;
        update_sql << "UPDATE orders SET status = '" << order_status_to_string(new_status) << "' WHERE id = " << order_id << ";";
        
        if (!db_.execute_non_query(update_sql.str())) {
            db_.rollback_transaction();
            return false;
        }
        
        // 提交事务
        if (!db_.commit_transaction()) {
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "更新订单状态失败: " << e.what() << std::endl;
        db_.rollback_transaction();
        return false;
    }
}

int OrderService::get_order_count(OrderStatus status, const std::string& start_date, const std::string& end_date) {
    int count = 0;
    
    // 构建 SQL 语句
    std::stringstream sql;
    sql << "SELECT COUNT(*) AS count FROM orders WHERE 1=1 ";
    
    // 添加状态过滤
    sql << "AND status = '" << order_status_to_string(status) << "' ";
    
    // 添加日期过滤
    if (!start_date.empty()) {
        sql << "AND DATE(created_at) >= '" << start_date << "' ";
    }
    
    if (!end_date.empty()) {
        sql << "AND DATE(created_at) <= '" << end_date << "' ";
    }
    
    // 执行查询
    auto result = db_.execute_query(sql.str());
    
    // 获取计数
    if (!result.empty()) {
        count = std::stoi(result[0]["count"]);
    }
    
    return count;
}

OrderStatus OrderService::string_to_order_status(const std::string& status_str) {
    if (status_str == "PENDING") {
        return OrderStatus::PENDING;
    } else if (status_str == "PAID") {
        return OrderStatus::PAID;
    } else if (status_str == "CANCELLED") {
        return OrderStatus::CANCELLED;
    } else if (status_str == "SHIPPED") {
        return OrderStatus::SHIPPED;
    } else {
        return OrderStatus::PENDING; // 默认返回 PENDING
    }
}

std::string OrderService::order_status_to_string(OrderStatus status) const {
    switch (status) {
        case OrderStatus::PENDING:
            return "PENDING";
        case OrderStatus::PAID:
            return "PAID";
        case OrderStatus::CANCELLED:
            return "CANCELLED";
        case OrderStatus::SHIPPED:
            return "SHIPPED";
        default:
            return "PENDING";
    }
}