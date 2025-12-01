# 个人记账与预算分析后端服务

一个使用现代C++实现的个人记账与预算分析后端服务，提供RESTful API接口，支持账户管理、分类管理、交易记录、预算管理和统计分析等功能。

## 技术栈

- **C++17**：使用现代C++特性实现
- **Boost.Beast**：HTTP服务器和客户端库
- **SQLite3**：轻量级关系型数据库
- **nlohmann/json**：JSON解析和序列化库
- **CMake**：项目构建工具
- **Catch2**：单元测试框架

## 项目结构

```
.
├── CMakeLists.txt          # CMake构建配置文件
├── README.md                # 项目说明文档
├── src/                     # 源代码目录
│   ├── main.cpp            # 程序入口点
│   ├── controllers/         # 控制器层
│   │   ├── AccountController.cpp
│   │   ├── AccountController.h
│   │   ├── CategoryController.cpp
│   │   ├── CategoryController.h
│   │   ├── TransactionController.cpp
│   │   ├── TransactionController.h
│   │   ├── BudgetController.cpp
│   │   ├── BudgetController.h
│   │   ├── SummaryController.cpp
│   │   └── SummaryController.h
│   ├── services/            # 服务层
│   │   ├── AccountService.cpp
│   │   ├── AccountService.h
│   │   ├── CategoryService.cpp
│   │   ├── CategoryService.h
│   │   ├── TransactionService.cpp
│   │   ├── TransactionService.h
│   │   ├── BudgetService.cpp
│   │   ├── BudgetService.h
│   │   ├── SummaryService.cpp
│   │   └── SummaryService.h
│   ├── dao/                 # 数据访问层
│   │   ├── Database.cpp
│   │   ├── Database.h
│   │   ├── AccountDAO.cpp
│   │   ├── AccountDAO.h
│   │   ├── CategoryDAO.cpp
│   │   ├── CategoryDAO.h
│   │   ├── TransactionDAO.cpp
│   │   ├── TransactionDAO.h
│   │   ├── BudgetDAO.cpp
│   │   └── BudgetDAO.h
│   ├── models/              # 数据模型层
│   │   ├── Account.h
│   │   ├── Category.h
│   │   ├── Transaction.h
│   │   └── Budget.h
│   └── utils/               # 工具类目录
└── tests/                   # 单元测试目录
```

## 构建步骤

1. **安装依赖项**：
   - Boost 1.70+：`sudo apt-get install libboost-all-dev` (Ubuntu)
   - SQLite3：`sudo apt-get install libsqlite3-dev` (Ubuntu)
   - nlohmann/json：通过CMake FetchContent自动下载
   - Catch2：通过CMake FetchContent自动下载

2. **配置CMake**：
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

3. **编译项目**：
   ```bash
   make
   ```

4. **运行单元测试**：
   ```bash
   make test
   ```

## 启动服务

```bash
./accounting_server [port]
```

- `port`：可选参数，指定服务端口，默认8080

示例：
```bash
./accounting_server 8081
```

## API接口

所有接口的请求和响应均使用JSON格式。

### 1. 账户管理

#### 创建账户
- **方法**：POST /accounts
- **请求体**：
  ```json
  {
    "name": "招商银行储蓄卡",
    "type": "bank",
    "initial_balance": 10000.00
  }
  ```
- **响应**：
  ```json
  {
    "id": 1,
    "name": "招商银行储蓄卡",
    "type": "bank",
    "initial_balance": 10000.00
  }
  ```

#### 查询账户列表
- **方法**：GET /accounts
- **查询参数**：
  - `type`：可选，按账户类型过滤
  - `page`：可选，页码，默认1
  - `page_size`：可选，每页大小，默认10
- **响应**：
  ```json
  {
    "accounts": [
      {
        "id": 1,
        "name": "招商银行储蓄卡",
        "type": "bank",
        "initial_balance": 10000.00
      }
    ],
    "total_count": 1,
    "page": 1,
    "page_size": 10
  }
  ```

#### 更新账户信息
- **方法**：PUT /accounts/{id}
- **请求体**：
  ```json
  {
    "name": "招商银行储蓄卡(更新)",
    "type": "bank"
  }
  ```
- **响应**：
  ```json
  {
    "id": 1,
    "name": "招商银行储蓄卡(更新)",
    "type": "bank",
    "initial_balance": 10000.00
  }
  ```

#### 删除账户
- **方法**：DELETE /accounts/{id}
- **响应**：
  ```json
  {
    "message": "Account deleted successfully"
  }
  ```

### 2. 分类管理

#### 创建分类
- **方法**：POST /categories
- **请求体**：
  ```json
  {
    "name": "餐饮",
    "type": "expense"
  }
  ```
- **响应**：
  ```json
  {
    "id": 1,
    "name": "餐饮",
    "type": "expense"
  }
  ```

#### 查询分类列表
- **方法**：GET /categories
- **响应**：
  ```json
  {
    "categories": [
      {
        "id": 1,
        "name": "餐饮",
        "type": "expense"
      }
    ]
  }
  ```

#### 更新分类
- **方法**：PUT /categories/{id}
- **请求体**：
  ```json
  {
    "name": "餐饮(更新)",
    "type": "expense"
  }
  ```
- **响应**：
  ```json
  {
    "id": 1,
    "name": "餐饮(更新)",
    "type": "expense"
  }
  ```

#### 删除分类
- **方法**：DELETE /categories/{id}
- **响应**：
  ```json
  {
    "message": "Category deleted successfully"
  }
  ```

### 3. 交易记录

#### 新增交易
- **方法**：POST /transactions
- **请求体**：
  ```json
  {
    "account_id": 1,
    "category_id": 1,
    "type": "expense",
    "amount": 50.00,
    "time": "2024-11-28T12:30:00",
    "note": "午餐"
  }
  ```
- **响应**：
  ```json
  {
    "id": 1,
    "account_id": 1,
    "category_id": 1,
    "type": "expense",
    "amount": 50.00,
    "time": "2024-11-28T12:30:00",
    "note": "午餐"
  }
  ```

#### 查询交易列表
- **方法**：GET /transactions
- **查询参数**：
  - `from`：可选，开始时间（ISO8601格式）
  - `to`：可选，结束时间（ISO8601格式）
  - `category_id`：可选，分类ID
  - `account_id`：可选，账户ID
  - `type`：可选，交易类型（income/expense）
  - `amount_min`：可选，最小金额
  - `amount_max`：可选，最大金额
  - `page`：可选，页码，默认1
  - `page_size`：可选，每页大小，默认10
  - `sort_by`：可选，排序字段（time/amount），默认time
  - `sort_order`：可选，排序顺序（asc/desc），默认desc
- **响应**：
  ```json
  {
    "transactions": [
      {
        "id": 1,
        "account_id": 1,
        "category_id": 1,
        "type": "expense",
        "amount": 50.00,
        "time": "2024-11-28T12:30:00",
        "note": "午餐"
      }
    ],
    "total_count": 1,
    "page": 1,
    "page_size": 10
  }
  ```

#### 更新交易
- **方法**：PUT /transactions/{id}
- **请求体**：
  ```json
  {
    "account_id": 1,
    "category_id": 1,
    "type": "expense",
    "amount": 60.00,
    "time": "2024-11-28T12:30:00",
    "note": "午餐(更新)"
  }
  ```
- **响应**：
  ```json
  {
    "id": 1,
    "account_id": 1,
    "category_id": 1,
    "type": "expense",
    "amount": 60.00,
    "time": "2024-11-28T12:30:00",
    "note": "午餐(更新)"
  }
  ```

#### 删除交易
- **方法**：DELETE /transactions/{id}
- **响应**：
  ```json
  {
    "message": "Transaction deleted successfully"
  }
  ```

### 4. 预算管理

#### 设置/更新某个月的预算
- **方法**：PUT /budgets
- **请求体**：
  ```json
  {
    "month": "2024-11",
    "items": [
      {
        "category_id": 1,
        "limit": 1500.00
      }
    ]
  }
  ```
- **响应**：
  ```json
  {
    "message": "Budget set successfully"
  }
  ```

#### 查询某个月的预算设置
- **方法**：GET /budgets?month=2024-11
- **响应**：
  ```json
  {
    "month": "2024-11",
    "budgets": [
      {
        "id": 1,
        "category_id": 1,
        "limit": 1500.00
      }
    ]
  }
  ```

### 5. 统计与分析

#### 月度汇总
- **方法**：GET /summary/monthly?month=2024-11
- **响应**：
  ```json
  {
    "month": "2024-11",
    "total_income": 10000.00,
    "total_expense": 50.00,
    "balance": 9950.00,
    "per_category": [
      {
        "category_id": 1,
        "category_name": "餐饮",
        "expense": 50.00,
        "budget_limit": 1500.00,
        "exceed": false
      }
    ]
  }
  ```

#### 趋势统计
- **方法**：GET /summary/trend?from=2024-01&to=2024-12
- **响应**：
  ```json
  {
    "from": "2024-01",
    "to": "2024-12",
    "trend_data": [
      {
        "month": "2024-01",
        "total_income": 10000.00,
        "total_expense": 5000.00
      },
      {
        "month": "2024-02",
        "total_income": 10000.00,
        "total_expense": 6000.00
      }
    ]
  }
  ```

## 性能优化

1. **数据库连接复用**：使用单例模式管理数据库连接，避免每次请求都打开/关闭数据库。

2. **缓存设计**：对月度汇总接口实现内存缓存，以month为key缓存统计结果，当该月相关的交易或预算发生变化时自动失效缓存。

3. **索引优化**：在数据库中对常用查询字段（如time、account_id、category_id等）增加索引，提高查询性能。

4. **并发处理**：使用Boost.Asio的多线程机制处理HTTP请求，支持基本的并发访问。

## 错误处理

所有接口在失败时返回规范的错误响应，包括：
- HTTP状态码：4xx（客户端错误）或5xx（服务器错误）
- JSON体包含：
  - `code`：应用级错误码（如"INVALID_PARAM"、"NOT_FOUND"、"INTERNAL_ERROR"等）
  - `message`：人类可读的错误信息

## 许可证

MIT License
