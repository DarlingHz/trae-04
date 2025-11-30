#ifndef PRODUCT_H
#define PRODUCT_H

#include <string>
#include <vector>
#include "database.h"

// 商品数据结构
struct Product {
    int id;
    std::string name;
    std::string sku;
    double price;
    int stock;
    int reorder_threshold;
    std::string created_at;
    std::string updated_at;
};

// 库存调整请求
struct StockAdjustRequest {
    int delta;
    std::string reason;
};

// 库存变动日志
struct StockLog {
    int id;
    int product_id;
    int delta;
    std::string reason;
    int previous_stock;
    int new_stock;
    std::string created_at;
};

// 商品服务接口
class ProductService {
public:
    ProductService(Database& db);
    
    // 创建商品
    Product create_product(const std::string& name, const std::string& sku, double price, int initial_stock, int reorder_threshold);
    
    // 获取商品列表（分页）
    std::vector<Product> get_products(int page, int page_size, const std::string& keyword = "");
    
    // 获取单个商品
    Product get_product(int id);
    
    // 更新商品
    Product update_product(int id, const std::string& name, double price, int reorder_threshold);
    
    // 调整库存
    bool adjust_stock(int product_id, int delta, const std::string& reason);
    
    // 获取低库存商品
    std::vector<Product> get_low_stock_products();
    
    // 获取商品总数
    int get_product_count(const std::string& keyword = "");
    
private:
    Database& db_;
};

#endif // PRODUCT_H