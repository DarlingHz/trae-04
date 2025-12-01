# 个人知识卡片管理系统

一个使用C++开发的个人知识卡片管理后端服务，支持用户认证、卡片管理、标签管理、搜索功能等。

## 功能特性

- **用户认证**：注册、登录、JWT令牌验证
- **卡片管理**：创建、编辑、删除、查询卡片
- **标签管理**：创建、编辑、删除、查询标签
- **搜索功能**：支持按标题、内容搜索卡片
- **标签过滤**：支持按标签过滤卡片
- **分页功能**：支持卡片列表分页
- **排序功能**：支持按创建时间、更新时间排序

## 技术栈

- **C++**：核心开发语言
- **SQLite**：数据库存储
- **SQLiteCpp**：SQLite C++封装库
- **nlohmann/json**：JSON处理库
- **cpp-httplib**：HTTP服务器库
- **jwt-cpp**：JWT令牌处理库
- **bcrypt**：密码哈希库
- **CMake**：项目构建工具

## 项目结构

```
.
├── src/                    # 源代码目录
│   ├── auth/              # 认证相关代码
│   ├── dao/               # 数据访问对象
│   ├── model/             # 数据模型
│   ├── service/           # 业务逻辑层
│   ├── util/              # 工具类
│   └── main.cpp           # 项目入口
├── include/                # 头文件目录
├── test/                   # 测试代码目录
│   ├── unit/               # 单元测试
│   └── integration/        # 集成测试
├── config/                 # 配置文件目录
├── data/                   # 数据文件目录
├── CMakeLists.txt          # CMake配置文件
└── README.md               # 项目说明文档
```

## 安装指南

### 环境要求

- C++17或更高版本
- CMake 3.10或更高版本
- SQLite 3.0或更高版本

### 依赖安装

1. **SQLiteCpp**：
   ```bash
   git clone https://github.com/SRombauts/SQLiteCpp.git
   cd SQLiteCpp
   mkdir build && cd build
   cmake ..
   make
   sudo make install
   ```

2. **nlohmann/json**：
   ```bash
   git clone https://github.com/nlohmann/json.git
   cd json
   mkdir build && cd build
   cmake ..
   make
   sudo make install
   ```

3. **cpp-httplib**：
   ```bash
   git clone https://github.com/yhirose/cpp-httplib.git
   cd cpp-httplib
   mkdir build && cd build
   cmake ..
   make
   sudo make install
   ```

4. **jwt-cpp**：
   ```bash
   git clone https://github.com/Thalhammer/jwt-cpp.git
   cd jwt-cpp
   mkdir build && cd build
   cmake ..
   make
   sudo make install
   ```

5. **bcrypt**：
   ```bash
   git clone https://github.com/rg3/bcrypt.git
   cd bcrypt
   make
   sudo make install
   ```

### 项目构建

1. 克隆项目：
   ```bash
   git clone <项目地址>
   cd <项目目录>
   ```

2. 创建构建目录：
   ```bash
   mkdir build && cd build
   ```

3. 配置CMake：
   ```bash
   cmake ..
   ```

4. 编译项目：
   ```bash
   make
   ```

5. 安装项目：
   ```bash
   sudo make install
   ```

## 使用说明

### 启动服务器

```bash
./knowledge-card-server
```

服务器将在 `http://localhost:8080` 启动。

### API接口

#### 用户认证

- **注册**：`POST /api/register`
  ```json
  {
    "email": "user@example.com",
    "password": "password123"
  }
  ```

- **登录**：`POST /api/login`
  ```json
  {
    "email": "user@example.com",
    "password": "password123"
  }
  ```

#### 卡片管理

- **创建卡片**：`POST /api/cards`
  ```json
  {
    "title": "卡片标题",
    "content": "卡片内容",
    "tags": ["标签1", "标签2"]
  }
  ```

- **获取卡片列表**：`GET /api/cards`
  - 参数：`page`（页码）、`page_size`（每页数量）、`sort`（排序字段）、`tags`（标签过滤）、`search`（搜索关键词）

- **获取卡片详情**：`GET /api/cards/{card_id}`

- **更新卡片**：`PUT /api/cards/{card_id}`
  ```json
  {
    "title": "更新后的标题",
    "content": "更新后的内容",
    "tags": ["更新后的标签"]
  }
  ```

- **删除卡片**：`DELETE /api/cards/{card_id}`

#### 标签管理

- **创建标签**：`POST /api/tags`
  ```json
  {
    "name": "标签名称"
  }
  ```

- **获取标签列表**：`GET /api/tags`

- **更新标签**：`PUT /api/tags/{tag_id}`
  ```json
  {
    "name": "更新后的标签名称"
  }
  ```

- **删除标签**：`DELETE /api/tags/{tag_id}`

### 认证方式

所有需要认证的接口都需要在请求头中包含 `Authorization` 字段，格式为：

```
Authorization: Bearer <JWT令牌>
```

## 测试

### 单元测试

```bash
cd build
make test
```

### 集成测试

集成测试需要启动服务器，然后运行测试脚本：

```bash
./knowledge-card-server &
python test/integration/test_api.py
```

## 配置文件

配置文件位于 `config/config.json`，可以配置以下参数：

```json
{
  "server": {
    "host": "localhost",
    "port": 8080
  },
  "database": {
    "path": "data/db.sqlite3"
  },
  "jwt": {
    "secret": "secret_key",
    "expiration": 86400
  }
}
```

## 许可证

MIT License
