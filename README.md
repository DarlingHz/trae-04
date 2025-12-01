# 共享出行平台后端系统

一个基于C++和Crow框架开发的共享出行平台后端系统，实现了乘客、车主、行程请求和行程管理等核心功能。

## 技术栈

- **编程语言**: C++17
- **Web框架**: Crow (https://github.com/CrowCpp/Crow)
- **数据库**: SQLite3
- **JSON处理**: nlohmann/json
- **构建工具**: CMake

## 项目结构

```
ride-sharing-app/
├── CMakeLists.txt          # CMake构建配置文件
├── README.md               # 项目说明文档
├── build/                  # 构建输出目录
├── config/                 # 配置文件目录
│   └── app.conf            # 应用程序配置文件
├── include/                # 头文件目录
│   ├── http/               # HTTP相关头文件
│   │   └── controllers/    # 控制器头文件
│   ├── model/              # 数据模型头文件
│   ├── repository/         # 数据访问层头文件
│   ├── service/            # 业务逻辑层头文件
│   └── utils/              # 工具类头文件
├── src/                     # 源文件目录
│   ├── http/               # HTTP相关源文件
│   │   └── controllers/    # 控制器源文件
│   ├── model/              # 数据模型源文件
│   ├── repository/         # 数据访问层源文件
│   ├── service/            # 业务逻辑层源文件
│   ├── utils/              # 工具类源文件
│   └── main.cpp            # 主函数文件
└── tests/                   # 测试文件目录
```

## 核心功能

### 1. 乘客管理
- 乘客注册
- 乘客信息查询
- 乘客信息更新
- 乘客删除

### 2. 车主管理
- 车主注册
- 车主信息查询
- 车主信息更新
- 车主状态更新
- 车主位置更新
- 车主删除

### 3. 行程请求管理
- 行程请求创建
- 行程请求查询
- 行程请求更新
- 行程请求状态更新
- 行程请求取消
- 行程请求删除

### 4. 行程管理
- 行程创建
- 行程查询
- 行程更新
- 行程状态更新
- 行程开始
- 行程结束
- 行程取消
- 行程删除

### 5. 匹配服务
- 乘客行程请求与车主的自动匹配
- 基于距离的匹配算法
- 匹配统计信息

## 安装和运行

### 1. 安装依赖

#### macOS
```bash
brew install cmake
brew install sqlite3
brew install nlohmann-json
```

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install cmake
sudo apt-get install libsqlite3-dev
sudo apt-get install nlohmann-json3-dev
```

### 2. 克隆项目

```bash
git clone <repository-url>
cd ride-sharing-app
```

### 3. 构建项目

```bash
mkdir build
cd build
cmake ..
make
```

### 4. 运行项目

```bash
./ride-sharing-app
```

服务器将在默认端口8080上启动。您可以通过访问http://localhost:8080来验证服务器是否正常运行。

## API文档

### 乘客相关API

#### 注册乘客
```
POST /riders
Content-Type: application/json

{
  "name": "张三",
  "phone": "13800138000"
}
```

#### 获取乘客信息
```
GET /riders/{id}
```

### 车主相关API

#### 注册车主
```
POST /drivers
Content-Type: application/json

{
  "name": "李四",
  "license_plate": "粤B12345",
  "car_model": "特斯拉Model 3",
  "capacity": 4
}
```

#### 获取车主信息
```
GET /drivers/{id}
```

### 行程请求相关API

#### 创建行程请求
```
POST /ride-requests
Content-Type: application/json

{
  "rider_id": 1,
  "start_x": 10,
  "start_y": 20,
  "end_x": 30,
  "end_y": 40
}
```

### 行程相关API

#### 开始行程
```
POST /trips/{id}/start
```

#### 结束行程
```
POST /trips/{id}/end
Content-Type: application/json

{
  "fare": 50.0
}
```

### 匹配服务相关API

#### 获取匹配统计信息
```
GET /matching/stats
```

## 配置文件

应用程序配置文件位于`config/app.conf`，您可以根据需要修改以下配置项：

```
# 服务器配置
server.port = 8080

# 数据库配置
database.path = ride-sharing.db

# 日志配置
log.level = DEBUG
log.file = ride-sharing-app.log

# 匹配服务配置
matching.radius = 100
```

## 日志

应用程序日志将记录到`ride-sharing-app.log`文件中。日志级别可以通过配置文件进行修改，默认级别为DEBUG。

## 数据库

应用程序使用SQLite3数据库，数据库文件将自动创建在项目根目录下的`ride-sharing.db`文件中。

## 测试

项目包含一个测试目录`tests/`，您可以使用以下命令运行测试：

```bash
cd build
make test
```

## 贡献

欢迎提交问题和拉取请求！

## 许可证

本项目采用MIT许可证。
