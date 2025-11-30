#include "product.h"
#include "database.h"
#include <iostream>
#include <sstream>
#include <algorithm>

ProductService::ProductService(Database& db) : db_(db) {
}

Product ProductService::create_product(const std::string& name, const std::string& sku, double price, int initial_stock, int reorder_threshold) {
    Product product;
    
    // 构建 SQL 语句
    std::stringstream sql;
    sql << "INSERT INTO products (name, sku, price, stock, reorder_threshold) "
        << "VALUES ('" << name << "', '" << sku << "', " << price << ", " << initial_stock << ", " << reorder_threshold << ");";
    
    if (db_.execute_non_query(sql.str())) {
        int product_id = static_cast<int>(db_.get_last_insert_rowid());
        product = get_product(product_id);
        
        // 记录库存变动日志
        std::stringstream log_sql;
        log_sql << "INSERT INTO stock_logs (product_id, delta, reason, previous_stock, new_stock) "
                << "VALUES (" << product_id << ", " << initial_stock << ", 'initial_stock', 0, " << initial_stock << ");";
        db_.execute_non_query(log_sql.str());
    }
    
    return product;
}

std::vector<Product> ProductService::get_products(int page, int page_size, const std::string& keyword) {
    std::vector<Product> products;
    
    // 计算偏移量
    int offset = (page - 1) * page_size;
    
    // 构建 SQL 语句
    std::stringstream sql;
    sql << "SELECT * FROM products ";
    
    if (!keyword.empty()) {
        sql << "WHERE name LIKE '%" << keyword << "%' OR sku LIKE '%" << keyword << "%' ";
    }
    
    sql << "ORDER BY created_at DESC LIMIT " << page_size << " OFFSET " << offset << ";";
    
    // 执行查询
    auto result = db_.execute_query(sql.str());
    
    // 转换结果为 Product 对象
    for (const auto& row : result) {
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
        
        products.push_back(product);
    }
    
    return products;
}

Product ProductService::get_product(int id) {
    Product product;
    product.id = -1; // 初始化为无效 ID
    
    // 构建 SQL 语句
    std::stringstream sql;
    sql << "SELECT * FROM products WHERE id = " << id << ";";
    
    // 执行查询
    auto result = db_.execute_query(sql.str());
    
    // 转换结果为 Product 对象
    if (!result.empty()) {
        const auto& row = result[0];
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
    }
    
    return product;
}

Product ProductService::update_product(int id, const std::string& name, double price, int reorder_threshold) {
    Product product;
    
    // 构建 SQL 语句
    std::stringstream sql;
    sql << "UPDATE products SET name = '" << name << "', price = " << price << ", reorder_threshold = " << reorder_threshold << " WHERE id = " << id << ";";
    
    if (db_.execute_non_query(sql.str())) {
        product = get_product(id);
    }
    
    return product;
}

bool ProductService::adjust_stock(int product_id, int delta, const std::string& reason) {
    // 开始事务
    if (!db_.begin_transaction()) {
        return false;
    }
    
    try {
        // 获取当前库存
        Product product = get_product(product_id);
        if (product.id == -1) {
            db_.rollback_transaction();
            return false;
        }
        
        // 计算新库存
        int new_stock = product.stock + delta;
        
        // 检查库存是否为负数
        if (new_stock < 0) {
            db_.rollback_transaction();
            return false;
        }
        
        // 更新库存
        std::stringstream update_sql;
        update_sql << "UPDATE products SET stock = " << new_stock << " WHERE id = " << product_id << ";";
        
        if (!db_.execute_non_query(update_sql.str())) {
            db_.rollback_transaction();
            return false;
        }
        
        // 记录库存变动日志
        std::stringstream log_sql;
        log_sql << "INSERT INTO stock_logs (product_id, delta, reason, previous_stock, new_stock) "
                << "VALUES (" << product_id << ", " << delta << ", '" << reason << "', " << product.stock << ", " << new_stock << ");";
        
        if (!db_.execute_non_query(log_sql.str())) {
            db_.rollback_transaction();
            return false;
        }
        
        // 提交事务
        if (!db_.commit_transaction()) {
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "库存调整失败: " << e.what() << std::endl;
        db_.rollback_transaction();
        return false;
    }
}

std::vector<Product> ProductService::get_low_stock_products() {
    std::vector<Product> products;
    
    // 构建 SQL 语句
    std::string sql = "SELECT * FROM products WHERE stock <= reorder_threshold ORDER BY stock ASC;";
    
    // 执行查询
    auto result = db_.execute_query(sql);
    
    // 转换结果为 Product 对象
    for (const auto& row : result) {
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
        
        products.push_back(product);
    }
    
    return products;
}

int ProductService::get_product_count(const std::string& keyword) {
    int count = 0;
    
    // 构建 SQL 语句
    std::stringstream sql;
    sql << "SELECT COUNT(*) AS count FROM products ";
    
    if (!keyword.empty()) {
        sql << "WHERE name LIKE '%" << keyword << "%' OR sku LIKE '%" << keyword << "%' ";
    }
    
    // 执行查询
    auto result = db_.execute_query(sql.str());
    
    // 获取计数
    if (!result.empty()) {
        count = std::stoi(result[0]["count"]);
    }
    
    return count;
}