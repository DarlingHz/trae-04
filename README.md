# 团队任务与日程管理系统

## 项目简介

这是一个基于C++17开发的团队任务与日程管理后端服务，提供HTTP/JSON接口，支持用户管理、项目管理、任务管理、统计查询等功能。

## 技术栈

- **C++17**：核心开发语言
- **Boost.Asio**：HTTP服务器实现
- **SQLite3**：持久化存储
- **nlohmann_json**：JSON解析与序列化
- **GoogleTest**：单元测试框架
- **CMake**：构建系统

## 整体架构

系统采用分层架构设计，主要分为以下几层：

1. **网络层**：基于Boost.Asio实现的HTTP服务器，处理客户端请求与响应
2. **业务逻辑层**：处理用户认证、权限控制、任务管理等核心业务逻辑
3. **数据访问层**：封装数据库操作，提供数据持久化与查询功能
4. **工具层**：提供JSON解析、密码哈希、Token生成等通用工具函数

## 环境依赖

- CMake 3.15及以上
- C++17兼容的编译器（如GCC 8+、Clang 7+）
- Boost 1.70及以上
- SQLite3 3.24及以上
- nlohmann_json 3.7及以上
- GoogleTest 1.10及以上

## 构建步骤

1. **克隆仓库**：
   ```bash
   git clone <repository-url>
   cd team_task_manager
   ```

2. **创建构建目录**：
   ```bash
   mkdir build
   cd build
   ```

3. **配置CMake**：
   ```bash
   cmake ..
   ```

4. **编译项目**：
   ```bash
   make
   ```

5. **初始化数据库**：
   ```bash
   sqlite3 task_manager.db < ../db/schema.sql
   ```

6. **启动服务**：
   ```bash
   ./team_task_manager
   ```

服务启动后将监听本地8080端口。

## API文档

### 统一响应结构

**成功响应**：
```json
{
  "code": 0,
  "message": "ok",
  "data": {...}
}
```

**失败响应**：
```json
{
  "code": <错误码>,
  "message": "错误原因",
  "data": null
}
```

### 用户相关接口

#### 注册接口
- **HTTP方法**：POST
- **URL**：/api/v1/users/register
- **请求体**：
  ```json
  {
    "name": "张三",
    "email": "zhangsan@example.com",
    "password": "123456"
  }
  ```
- **响应体**：
  ```json
  {
    "code": 0,
    "message": "ok",
    "data": {
      "id": 1,
      "name": "张三",
      "email": "zhangsan@example.com",
      "access_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
    }
  }
  ```

#### 登录接口
- **HTTP方法**：POST
- **URL**：/api/v1/users/login
- **请求体**：
  ```json
  {
    "email": "zhangsan@example.com",
    "password": "123456"
  }
  ```
- **响应体**：
  ```json
  {
    "code": 0,
    "message": "ok",
    "data": {
      "id": 1,
      "name": "张三",
      "email": "zhangsan@example.com",
      "access_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
    }
  }
  ```

### 项目管理接口

#### 创建项目
- **HTTP方法**：POST
- **URL**：/api/v1/projects
- **请求头**：Authorization: Bearer <access_token>
- **请求体**：
  ```json
  {
    "name": "项目A",
    "description": "这是项目A的描述"
  }
  ```
- **响应体**：
  ```json
  {
    "code": 0,
    "message": "ok",
    "data": {
      "id": 1,
      "name": "项目A",
      "description": "这是项目A的描述",
      "owner_user_id": 1,
      "created_at": "2023-01-01T00:00:00Z"
    }
  }
  ```

#### 列出项目
- **HTTP方法**：GET
- **URL**：/api/v1/projects?page=1&page_size=10
- **请求头**：Authorization: Bearer <access_token>
- **响应体**：
  ```json
  {
    "code": 0,
    "message": "ok",
    "data": {
      "projects": [
        {
          "id": 1,
          "name": "项目A",
          "description": "这是项目A的描述",
          "owner_user_id": 1,
          "created_at": "2023-01-01T00:00:00Z"
        }
      ],
      "total": 1,
      "page": 1,
      "page_size": 10
    }
  }
  ```

#### 查询项目详情
- **HTTP方法**：GET
- **URL**：/api/v1/projects/{project_id}
- **请求头**：Authorization: Bearer <access_token>
- **响应体**：
  ```json
  {
    "code": 0,
    "message": "ok",
    "data": {
      "id": 1,
      "name": "项目A",
      "description": "这是项目A的描述",
      "owner_user_id": 1,
      "created_at": "2023-01-01T00:00:00Z",
      "stats": {
        "total_tasks": 5,
        "todo_tasks": 3,
        "doing_tasks": 1,
        "done_tasks": 1
      }
    }
  }
  ```

### 任务管理接口

#### 创建任务
- **HTTP方法**：POST
- **URL**：/api/v1/projects/{project_id}/tasks
- **请求头**：Authorization: Bearer <access_token>
- **请求体**：
  ```json
  {
    "title": "任务1",
    "description": "这是任务1的描述",
    "assignee_user_id": 1,
    "status": "todo",
    "priority": "medium",
    "due_date": "2023-01-10T00:00:00Z",
    "tags": ["标签1", "标签2"]
  }
  ```
- **响应体**：
  ```json
  {
    "code": 0,
    "message": "ok",
    "data": {
      "id": 1,
      "project_id": 1,
      "title": "任务1",
      "description": "这是任务1的描述",
      "assignee_user_id": 1,
      "status": "todo",
      "priority": "medium",
      "due_date": "2023-01-10T00:00:00Z",
      "created_at": "2023-01-01T00:00:00Z",
      "updated_at": "2023-01-01T00:00:00Z",
      "tags": ["标签1", "标签2"]
    }
  }
  ```

#### 列出任务
- **HTTP方法**：GET
- **URL**：/api/v1/projects/{project_id}/tasks?status=todo&priority=medium&page=1&page_size=10
- **请求头**：Authorization: Bearer <access_token>
- **响应体**：
  ```json
  {
    "code": 0,
    "message": "ok",
    "data": {
      "tasks": [
        {
          "id": 1,
          "project_id": 1,
          "title": "任务1",
          "description": "这是任务1的描述",
          "assignee_user_id": 1,
          "status": "todo",
          "priority": "medium",
          "due_date": "2023-01-10T00:00:00Z",
          "created_at": "2023-01-01T00:00:00Z",
          "updated_at": "2023-01-01T00:00:00Z",
          "tags": ["标签1", "标签2"]
        }
      ],
      "total": 1,
      "page": 1,
      "page_size": 10
    }
  }
  ```

#### 查询任务详情
- **HTTP方法**：GET
- **URL**：/api/v1/tasks/{task_id}
- **请求头**：Authorization: Bearer <access_token>
- **响应体**：
  ```json
  {
    "code": 0,
    "message": "ok",
    "data": {
      "id": 1,
      "project_id": 1,
      "title": "任务1",
      "description": "这是任务1的描述",
      "assignee_user_id": 1,
      "status": "todo",
      "priority": "medium",
      "due_date": "2023-01-10T00:00:00Z",
      "created_at": "2023-01-01T00:00:00Z",
      "updated_at": "2023-01-01T00:00:00Z",
      "tags": ["标签1", "标签2"]
    }
  }
  ```

#### 更新任务
- **HTTP方法**：PATCH
- **URL**：/api/v1/tasks/{task_id}
- **请求头**：Authorization: Bearer <access_token>
- **请求体**：
  ```json
  {
    "status": "doing",
    "priority": "high"
  }
  ```
- **响应体**：
  ```json
  {
    "code": 0,
    "message": "ok",
    "data": {
      "id": 1,
      "project_id": 1,
      "title": "任务1",
      "description": "这是任务1的描述",
      "assignee_user_id": 1,
      "status": "doing",
      "priority": "high",
      "due_date": "2023-01-10T00:00:00Z",
      "created_at": "2023-01-01T00:00:00Z",
      "updated_at": "2023-01-02T00:00:00Z",
      "tags": ["标签1", "标签2"]
    }
  }
  ```

### 搜索与统计接口

#### 搜索任务
- **HTTP方法**：GET
- **URL**：/api/v1/tasks/search?keyword=任务&status=todo&tag=标签1&due_before=2023-01-15T00:00:00Z&page=1&page_size=10
- **请求头**：Authorization: Bearer <access_token>
- **响应体**：
  ```json
  {
    "code": 0,
    "message": "ok",
    "data": {
      "tasks": [
        {
          "id": 1,
          "project_id": 1,
          "title": "任务1",
          "description": "这是任务1的描述",
          "assignee_user_id": 1,
          "status": "todo",
          "priority": "medium",
          "due_date": "2023-01-10T00:00:00Z",
          "created_at": "2023-01-01T00:00:00Z",
          "updated_at": "2023-01-01T00:00:00Z",
          "tags": ["标签1", "标签2"]
        }
      ],
      "total": 1,
      "page": 1,
      "page_size": 10
    }
  }
  ```

#### 统计概览
- **HTTP方法**：GET
- **URL**：/api/v1/stats/overview
- **请求头**：Authorization: Bearer <access_token>
- **响应体**：
  ```json
  {
    "code": 0,
    "message": "ok",
    "data": {
      "task_stats": {
        "todo": 3,
        "doing": 1,
        "done": 1
      },
      "overdue_tasks": 0,
      "recent_tasks": 5,
      "total_projects": 2
    }
  }
  ```

### 审计日志接口

#### 查看审计日志
- **HTTP方法**：GET
- **URL**：/api/v1/audit_logs?limit=10
- **请求头**：Authorization: Bearer <access_token>
- **响应体**：
  ```json
  {
    "code": 0,
    "message": "ok",
    "data": {
      "logs": [
        {
          "id": 1,
          "user_id": 1,
          "action_type": "create_task",
          "resource_type": "task",
          "resource_id": 1,
          "created_at": "2023-01-01T00:00:00Z",
          "detail": "{\"title\": \"任务1\"}"
        }
      ]
    }
  }
  ```

## 性能与优化说明

### 数据库优化
1. **索引优化**：
   - 为Task表的status、due_date、assignee_user_id、project_id字段建立索引，提高查询效率
   - 为User表的email字段建立唯一索引，确保email的唯一性并提高登录查询效率
   - 为Project表的owner_user_id字段建立索引，提高项目列表查询效率

2. **数据库连接池**：
   - 实现简单的数据库连接池，避免每次请求都重新建立数据库连接，提高系统性能
   - 连接池大小可配置，默认大小为10

### 网络优化
1. **线程池**：
   - 使用Boost.Asio的线程池处理HTTP请求，提高系统的并发处理能力
   - 线程池大小可配置，默认大小为CPU核心数的2倍

2. **非阻塞I/O**：
   - 基于Boost.Asio实现非阻塞I/O，提高系统的I/O处理效率
   - 支持异步读写操作，减少线程阻塞时间

### 缓存优化
1. **用户信息缓存**：
   - 对当前用户的信息进行短期缓存（默认5分钟），减少数据库查询次数
   - 缓存以用户ID为键，存储用户的基本信息

2. **项目列表缓存**：
   - 对用户的项目列表进行短期缓存（默认10分钟），提高项目列表查询效率
   - 缓存以用户ID为键，存储用户有权限访问的项目列表

## 假设与限制

1. **密码哈希**：
   - 当前实现使用简单的SHA256哈希函数对密码进行加密，不适合生产环境
   - 生产环境建议使用更安全的密码哈希算法，如Argon2、bcrypt等

2. **Token认证**：
   - 当前实现使用简单的随机字符串作为access_token，不包含过期时间
   - 生产环境建议使用JWT（JSON Web Token），包含过期时间和签名，提高安全性

3. **数据库**：
   - 当前实现使用SQLite3作为数据库，适合本地开发和测试
   - 生产环境建议使用MySQL或PostgreSQL等更适合高并发场景的数据库

4. **并发处理**：
   - 当前实现使用线程池处理HTTP请求，并发处理能力有限
   - 生产环境建议使用负载均衡器和多进程部署，提高系统的并发处理能力

## 未来优化方向

1. **分布式缓存**：
   - 引入Redis等分布式缓存系统，提高缓存的扩展性和可靠性

2. **消息队列**：
   - 引入Kafka或RabbitMQ等消息队列系统，实现异步处理和削峰填谷

3. **微服务架构**：
   - 将系统拆分为多个微服务，提高系统的可扩展性和可维护性

4. **监控与日志**：
   - 引入Prometheus和Grafana等监控系统，实现系统性能监控和告警
   - 引入ELK Stack（Elasticsearch、Logstash、Kibana）等日志系统，实现日志收集、分析和可视化

5. **安全加固**：
   - 实现HTTPS加密传输，提高数据传输的安全性
   - 实现CSRF（跨站请求伪造）防护，提高系统的安全性
   - 实现SQL注入防护，提高系统的安全性

## 联系方式

如有任何问题或建议，请联系：
- 邮箱：support@example.com
- 电话：1234567890

## 许可证

本项目采用MIT许可证，详见LICENSE文件。