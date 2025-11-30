-- 团队任务与日程管理系统数据库建表脚本

-- 创建用户表
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    email TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- 创建项目表
CREATE TABLE IF NOT EXISTS projects (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    owner_user_id INTEGER NOT NULL,
    name TEXT NOT NULL,
    description TEXT,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (owner_user_id) REFERENCES users (id) ON DELETE CASCADE
);

-- 创建任务表
CREATE TABLE IF NOT EXISTS tasks (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    project_id INTEGER NOT NULL,
    assignee_user_id INTEGER,
    title TEXT NOT NULL,
    description TEXT,
    status TEXT NOT NULL DEFAULT 'todo',
    priority TEXT NOT NULL DEFAULT 'medium',
    due_date DATETIME,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (project_id) REFERENCES projects (id) ON DELETE CASCADE,
    FOREIGN KEY (assignee_user_id) REFERENCES users (id) ON DELETE SET NULL,
    CHECK (status IN ('todo', 'doing', 'done')),
    CHECK (priority IN ('low', 'medium', 'high'))
);

-- 创建标签表
CREATE TABLE IF NOT EXISTS tags (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE
);

-- 创建任务标签关联表
CREATE TABLE IF NOT EXISTS task_tags (
    task_id INTEGER NOT NULL,
    tag_id INTEGER NOT NULL,
    PRIMARY KEY (task_id, tag_id),
    FOREIGN KEY (task_id) REFERENCES tasks (id) ON DELETE CASCADE,
    FOREIGN KEY (tag_id) REFERENCES tags (id) ON DELETE CASCADE
);

-- 创建审计日志表
CREATE TABLE IF NOT EXISTS audit_logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    action_type TEXT NOT NULL,
    resource_type TEXT NOT NULL,
    resource_id INTEGER,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    detail TEXT,
    FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE
);

-- 创建索引以提高查询性能

-- 用户表索引
CREATE INDEX IF NOT EXISTS idx_users_email ON users (email);

-- 项目表索引
CREATE INDEX IF NOT EXISTS idx_projects_owner_user_id ON projects (owner_user_id);
CREATE INDEX IF NOT EXISTS idx_projects_name ON projects (name);

-- 任务表索引
CREATE INDEX IF NOT EXISTS idx_tasks_project_id ON tasks (project_id);
CREATE INDEX IF NOT EXISTS idx_tasks_assignee_user_id ON tasks (assignee_user_id);
CREATE INDEX IF NOT EXISTS idx_tasks_status ON tasks (status);
CREATE INDEX IF NOT EXISTS idx_tasks_priority ON tasks (priority);
CREATE INDEX IF NOT EXISTS idx_tasks_due_date ON tasks (due_date);
CREATE INDEX IF NOT EXISTS idx_tasks_created_at ON tasks (created_at);
CREATE INDEX IF NOT EXISTS idx_tasks_updated_at ON tasks (updated_at);

-- 标签表索引
CREATE INDEX IF NOT EXISTS idx_tags_name ON tags (name);

-- 任务标签关联表索引
CREATE INDEX IF NOT EXISTS idx_task_tags_tag_id ON task_tags (tag_id);

-- 审计日志表索引
CREATE INDEX IF NOT EXISTS idx_audit_logs_user_id ON audit_logs (user_id);
CREATE INDEX IF NOT EXISTS idx_audit_logs_action_type ON audit_logs (action_type);
CREATE INDEX IF NOT EXISTS idx_audit_logs_resource_type ON audit_logs (resource_type);
CREATE INDEX IF NOT EXISTS idx_audit_logs_created_at ON audit_logs (created_at);

-- 插入初始数据

-- 插入默认标签
INSERT OR IGNORE INTO tags (name) VALUES ('重要');
INSERT OR IGNORE INTO tags (name) VALUES ('紧急');
INSERT OR IGNORE INTO tags (name) VALUES ('开发');
INSERT OR IGNORE INTO tags (name) VALUES ('测试');
INSERT OR IGNORE INTO tags (name) VALUES ('文档');

-- 插入管理员用户（密码：admin123，哈希值：SHA256('admin123')）
INSERT OR IGNORE INTO users (name, email, password_hash) 
VALUES ('管理员', 'admin@example.com', '0192023a7bbd73250516f069df18b50');

-- 插入示例项目
INSERT OR IGNORE INTO projects (owner_user_id, name, description) 
VALUES (1, '示例项目', '这是一个示例项目，用于演示系统功能');

-- 插入示例任务
INSERT OR IGNORE INTO tasks (project_id, assignee_user_id, title, description, status, priority, due_date) 
VALUES (1, 1, '示例任务1', '这是示例任务1的描述', 'todo', 'high', '2023-12-31T23:59:59Z');

INSERT OR IGNORE INTO tasks (project_id, assignee_user_id, title, description, status, priority, due_date) 
VALUES (1, 1, '示例任务2', '这是示例任务2的描述', 'doing', 'medium', '2023-12-31T23:59:59Z');

INSERT OR IGNORE INTO tasks (project_id, assignee_user_id, title, description, status, priority, due_date) 
VALUES (1, 1, '示例任务3', '这是示例任务3的描述', 'done', 'low', '2023-12-31T23:59:59Z');

-- 为示例任务添加标签
INSERT OR IGNORE INTO task_tags (task_id, tag_id) VALUES (1, 1); -- 重要
INSERT OR IGNORE INTO task_tags (task_id, tag_id) VALUES (1, 2); -- 紧急
INSERT OR IGNORE INTO task_tags (task_id, tag_id) VALUES (2, 3); -- 开发
INSERT OR IGNORE INTO task_tags (task_id, tag_id) VALUES (3, 5); -- 文档

-- 插入示例审计日志
INSERT OR IGNORE INTO audit_logs (user_id, action_type, resource_type, resource_id, detail) 
VALUES (1, 'create_project', 'project', 1, '{"name": "示例项目"}');

INSERT OR IGNORE INTO audit_logs (user_id, action_type, resource_type, resource_id, detail) 
VALUES (1, 'create_task', 'task', 1, '{"title": "示例任务1"}');

INSERT OR IGNORE INTO audit_logs (user_id, action_type, resource_type, resource_id, detail) 
VALUES (1, 'create_task', 'task', 2, '{"title": "示例任务2"}');

INSERT OR IGNORE INTO audit_logs (user_id, action_type, resource_type, resource_id, detail) 
VALUES (1, 'create_task', 'task', 3, '{"title": "示例任务3"}');