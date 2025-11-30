# 库存与订单管理后端服务

这是一个使用 C++ 实现的库存与订单管理后端服务，提供 HTTP/JSON 接口。

## 功能特性

### 商品管理
- 创建商品
- 分页查询商品列表（支持模糊搜索）
- 查看单个商品详情
- 更新商品信息
- 调整商品库存
- 获取低库存商品列表

### 订单管理
- 创建订单（自动扣减库存）
- 查看订单详情
- 分页查询订单列表（支持按状态、时间范围过滤）
- 更新订单状态（支持合理的状态流转）
- 取消订单时支持回滚库存

### 统计功能
- 获取指定日期的订单统计数据

## 技术栈

- **C++17**：主要开发语言
- **SQLite**：关系型数据库
- **CMake**：项目构建工具
- **自定义 HTTP 服务器**：处理 HTTP 请求和响应
- **自定义 JSON 解析器**：解析和生成 JSON 数据

## 项目结构

```
.
├── CMakeLists.txt          # CMake 构建配置文件
├── README.md                # 项目说明文档
├── sql/                     # 数据库脚本目录
│   └── schema.sql           # 数据库表结构定义
├── src/                     # 源代码目录
│   ├── main.cpp             # 主函数，启动服务器
│   ├── server.cpp           # HTTP 服务器实现
│   ├── database.cpp         # 数据库连接和操作实现
│   ├── product_service.cpp  # 商品服务实现
│   ├── order_service.cpp    # 订单服务实现
│   └── stats_service.cpp    # 统计服务实现
├── include/                  # 头文件目录
│   ├── server.h             # HTTP 服务器接口定义
│   ├── database.h            # 数据库接口定义
│   ├── product.h             # 商品相关数据结构和服务接口定义
│   ├── order.h               # 订单相关数据结构和服务接口定义
│   └── stats.h               # 统计相关数据结构和服务接口定义
└── tests/                    # 测试目录
```

## 编译和运行

### 编译

1. 创建构建目录：
```bash
mkdir build
cd build
```

2. 配置 CMake：
```bash
cmake ..
```

3. 编译项目：
```bash
make
```

编译完成后，将在 `build` 目录下生成可执行文件 `inventory_order_management`。

### 运行

1. 初始化数据库：
```bash
# 执行建表 SQL
sqlite3 inventory_order_management.db < ../sql/schema.sql
```

2. 启动服务器：
```bash
./inventory_order_management
```

默认情况下，服务器将在端口 8080 上运行。您可以使用以下命令指定端口和数据库文件路径：

```bash
./inventory_order_management --port 8081 --db /path/to/database.db
```

使用 `--help` 选项查看所有可用的命令行参数：

```bash
./inventory_order_management --help
```

## API 接口使用示例

### 1. 创建商品

```bash
curl -X POST http://localhost:8080/products \
  -H "Content-Type: application/json" \
  -d '{
    "name": "Apple iPhone 15",
    "sku": "IP15-128-BLK",
    "price": 6999.0,
    "initial_stock": 100,
    "reorder_threshold": 10
  }'
```

### 2. 创建订单

```bash
curl -X POST http://localhost:8080/orders \
  -H "Content-Type: application/json" \
  -d '{
    "items": [
      { "product_id": 1, "quantity": 2 },
      { "product_id": 3, "quantity": 1 }
    ]
  }'
```

### 3. 更新订单状态

将订单状态从 PENDING 改为 PAID：

```bash
curl -X POST http://localhost:8080/orders/1/status \
  -H "Content-Type: application/json" \
  -d '{
    "status": "PAID"
  }'
```

取消订单并回滚库存：

```bash
curl -X POST http://localhost:8080/orders/1/status \
  -H "Content-Type: application/json" \
  -d '{
    "status": "CANCELLED",
    "restock": true
  }'
```

### 4. 获取商品列表

```bash
curl -X GET "http://localhost:8080/products?page=1&page_size=10&keyword=iPhone"
```

### 5. 获取订单列表

```bash
curl -X GET "http://localhost:8080/orders?page=1&page_size=10&status=PAID&start_date=2023-01-01&end_date=2023-01-31"
```

### 6. 获取低库存商品

```bash
curl -X GET http://localhost:8080/stats/low_stock
```

### 7. 获取每日统计数据

```bash
curl -X GET "http://localhost:8080/stats/daily_summary?date=2023-01-01"
```

## 性能优化

1. **数据库预编译语句**：虽然当前实现中没有显式使用预编译语句，但 SQLite 会自动缓存编译后的 SQL 语句，提高重复执行相同 SQL 的性能。

2. **批量操作**：在创建订单时，批量查询所有商品信息，避免多次单独查询数据库。

3. **索引优化**：为数据库表中的高频查询字段（如 sku、status、created_at 等）创建了索引，提高查询性能。

4. **事务处理**：在涉及多个数据库操作的业务逻辑（如创建订单、调整库存等）中使用事务，确保数据一致性，并提高多个操作的执行性能。

5. **减少不必要的拷贝**：在代码中合理使用 const& 和 std::move，减少不必要的对象拷贝，提高性能。

## 测试

项目提供了以下测试方式：

1. **API 测试**：使用 curl 命令测试所有 API 接口，确保它们能够正常工作。

2. **业务流测试**：
   - 创建商品 → 创建订单 → 修改订单状态 → 查询统计
   - 库存不足导致下单失败
   - 取消订单并回滚库存的场景

## 注意事项

1. 本项目使用 SQLite 数据库，适合小型应用和开发测试。如果需要更高的性能和并发支持，可以考虑使用 MySQL 或 PostgreSQL 数据库。

2. 本项目实现了一个简单的 HTTP 服务器，仅支持基本的 HTTP 方法和请求处理。如果需要更完善的 HTTP 服务器功能，可以考虑使用第三方库（如 Boost.Beast、cpp-httplib 等）。

3. 本项目的 JSON 解析器是一个简单的实现，仅支持基本的 JSON 结构和数据类型。如果需要更完善的 JSON 解析功能，可以考虑使用第三方库（如 nlohmann/json、RapidJSON 等）。

4. 本项目没有实现用户认证和授权功能。如果需要在生产环境中使用，建议添加用户认证和授权功能，以确保系统的安全性。