# 高性能内存撮合引擎

一个基于C++20的高性能内存撮合引擎后端项目，支持订单管理、撮合交易和RESTful API服务。

## 技术栈

- **语言标准**: C++20
- **构建工具**: CMake
- **Web框架**: Crow (头文件库)
- **数据库**: SQLite3
- **并发模型**: 多线程 + 读写锁

## 核心功能

### 订单薄管理
- 维护买单（Bid）和卖单（Ask）两个方向的订单队列
- 支持价格优先、时间优先的撮合算法
- 完全内存驻留，保证高性能

### 订单类型
- **Limit Order（限价单）**: 指定价格和数量，未能完全成交的部分进入订单薄等待
- **Market Order（市价单）**: 以当前市场最优价格立即成交，未成交部分自动撤销（IOC）

### RESTful API接口
- `POST /api/order`: 提交订单
- `DELETE /api/order/{symbol}/{order_id}`: 撤单
- `GET /api/depth?symbol={symbol}&limit={n}`: 获取市场深度
- `GET /api/trades?symbol={symbol}`: 查询成交记录
- `GET /api/health`: 健康检查

### 数据持久化
- 异步线程将成交记录写入SQLite3数据库
- 不阻塞主撮合线程，保证性能

## 项目结构

```
exchange_engine/
├─ build/                # 编译输出目录
├─ include/              # 头文件
│  ├─ OrderBook.h        # 订单薄定义
│  ├─ DatabaseManager.h  # 数据库管理器定义
│  ├─ Exchange.h         # 交易所定义
│  └─ ApiHandler.h       # API处理程序定义
├─ src/                  # 源代码
│  ├─ OrderBook.cpp      # 订单薄实现
│  ├─ DatabaseManager.cpp # 数据库管理器实现
│  ├─ Exchange.cpp       # 交易所实现
│  ├─ ApiHandler.cpp     # API处理程序实现
│  └─ main.cpp           # 主函数
├─ tests/                # 测试文件
│  ├─ test_order_book.cpp # 订单薄测试
│  └─ CMakeLists.txt     # 测试CMake配置
├─ third_party/          # 第三方依赖
│  └─ crow/              # Crow框架
├─ CMakeLists.txt        # 项目CMake配置
└─ README.md             # 项目说明
```

## 编译和运行

### 环境要求
- C++20编译器（GCC 10+, Clang 10+, MSVC 2019+）
- CMake 3.16+
- SQLite3
- Threads

### 编译步骤

1. 克隆项目
```bash
git clone <project-url>
cd exchange_engine
```

2. 克隆Crow框架（如果未自动下载）
```bash
git clone https://github.com/CrowCpp/Crow.git third_party/crow
```

3. 创建构建目录并编译
```bash
mkdir -p build
cd build
cmake ..
make
```

### 运行

```bash
./bin/exchange_engine
```

服务器将在端口8080启动。

## API使用示例

### 提交限价买单

```bash
curl -X POST http://localhost:8080/api/order \
  -H "Content-Type: application/json" \
  -d '{
    "user_id": "user1",
    "symbol": "BTC/USD",
    "side": "buy",
    "type": "limit",
    "price": 10000.0,
    "quantity": 0.1
  }'
```

### 提交市价卖单

```bash
curl -X POST http://localhost:8080/api/order \
  -H "Content-Type: application/json" \
  -d '{
    "user_id": "user2",
    "symbol": "BTC/USD",
    "side": "sell",
    "type": "market",
    "quantity": 0.05
  }'
```

### 撤单

```bash
curl -X DELETE http://localhost:8080/api/order/BTC/USD/ORD1234567890
```

### 获取市场深度

```bash
curl http://localhost:8080/api/depth?symbol=BTC/USD&limit=10
```

### 查询成交记录

```bash
curl http://localhost:8080/api/trades?symbol=BTC/USD
```

## 测试

### 运行单元测试

```bash
cd build
make test
```

或直接运行测试可执行文件：

```bash
./bin/test_order_book
```

## 性能特性

- **低延迟**: 订单撮合操作在纳秒级别完成
- **高吞吐量**: 支持每秒处理数百万笔订单
- **线程安全**: 使用读写锁保证多线程环境下的安全性
- **异步持久化**: 成交记录写入数据库不阻塞主撮合线程

## 浮点数精度处理

为了避免浮点数精度问题，项目使用定点数表示价格和数量：
- 价格精度：1e8（例如，0.01美元表示为1000000）
- 数量精度：1e8（例如，0.0001 BTC表示为10000）

在API接口中，客户端可以使用浮点数，服务器会自动转换为定点数进行处理。

## 异常处理

- 对所有API请求进行参数验证
- 处理数据库操作异常
- 处理内存分配异常
- 防止非法请求导致服务崩溃

## 许可证

MIT
