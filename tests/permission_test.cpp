// 权限校验测试

#include <gtest/gtest.h>
#include <stdexcept>

#include "task_service.h"
#include "project_service.h"
#include "database.h"
#include "config.h"
#include "utils.h"

class PermissionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化数据库
        database_ = std::make_shared<Database>(config::kDatabasePath, config::kDatabaseConnectionPoolSize);
        
        // 初始化认证服务
        auth_service_ = std::make_shared<AuthService>(database_);
        
        // 初始化任务服务和项目服务
        task_service_ = std::make_shared<TaskService>(database_, auth_service_);
        project_service_ = std::make_shared<ProjectService>(database_, auth_service_);
        
        // 创建用户1（所有者）（如果不存在）
        auto existing_user1 = database_->GetUserByEmail("user1_permission@example.com");
        if (existing_user1) {
            user1_id_ = existing_user1->id;
        } else {
            User user1;
            user1.name = "User 1";
            user1.email = "user1_permission@example.com";
            user1.password_hash = "hash1";
            
            if (database_->CreateUser(user1)) {
                auto new_user1 = database_->GetUserByEmail("user1_permission@example.com");
                if (new_user1) {
                    user1_id_ = new_user1->id;
                }
            }
        }
        
        // 创建用户2（非所有者）（如果不存在）
        auto existing_user2 = database_->GetUserByEmail("user2_permission@example.com");
        if (existing_user2) {
            user2_id_ = existing_user2->id;
        } else {
            User user2;
            user2.name = "User 2";
            user2.email = "user2_permission@example.com";
            user2.password_hash = "hash2";
            
            if (database_->CreateUser(user2)) {
                auto new_user2 = database_->GetUserByEmail("user2_permission@example.com");
                if (new_user2) {
                    user2_id_ = new_user2->id;
                }
            }
        }
        
        // 创建用户1的项目
        Project project;
        project.name = "User 1's Project";
        project.description = "Project owned by User 1";
        project.owner_user_id = user1_id_;
        
        project_id_ = database_->CreateProject(project);
        
        // 创建项目中的任务
        Task task;
        task.title = "Test Task";
        task.description = "Task in User 1's project";
        task.status = "todo";
        task.priority = "medium";
        task.project_id = project_id_;
        task.assignee_user_id = user1_id_;
        
        task_id_ = database_->CreateTask(task);
    }
    
    void TearDown() override {
        // 删除测试任务
        if (task_id_ > 0) {
            database_->DeleteTask(task_id_);
        }
        
        // 删除测试项目
        if (project_id_ > 0) {
            database_->DeleteProject(project_id_);
        }
        
        // 删除测试用户
        if (user1_id_ > 0) {
            database_->DeleteUser(user1_id_);
        }
        
        if (user2_id_ > 0) {
            database_->DeleteUser(user2_id_);
        }
    }
    
    std::shared_ptr<DatabaseConnectionPool> connection_pool_;
    std::shared_ptr<Database> database_;
    std::shared_ptr<AuthService> auth_service_;
    std::shared_ptr<TaskService> task_service_;
    std::shared_ptr<ProjectService> project_service_;
    
    int user1_id_ = 0;  // 项目所有者
    int user2_id_ = 0;  // 非所有者
    int project_id_ = 0;
    int task_id_ = 0;
};

// 测试用户1（所有者）可以访问自己的项目
TEST_F(PermissionTest, UserCanAccessOwnProject) {
    // 用户1查询自己的项目
    std::optional<Project> project_opt = project_service_->GetProjectById(user1_id_, project_id_);
    
    // 验证查询成功
    EXPECT_TRUE(project_opt.has_value());
    if (project_opt.has_value()) {
        Project project = project_opt.value();
        EXPECT_EQ(project.id, project_id_);
        EXPECT_EQ(project.owner_user_id, user1_id_);
    }
}

// 测试用户2（非所有者）无法访问用户1的项目
TEST_F(PermissionTest, UserCannotAccessOthersProject) {
    // 用户2尝试查询用户1的项目，应该抛出权限异常
    EXPECT_THROW(
        project_service_->GetProjectById(user2_id_, project_id_),
        ProjectServiceException
    );
}

// 测试用户1（所有者）可以更新自己的任务
TEST_F(PermissionTest, UserCanUpdateOwnTask) {
    // 用户1更新任务状态
    std::optional<Task> updated_task_opt = task_service_->UpdateTask(user1_id_, task_id_, std::nullopt, std::nullopt, std::nullopt, "doing", std::nullopt, std::nullopt, std::nullopt);
    
    // 验证任务状态已更新
    EXPECT_TRUE(updated_task_opt.has_value());
    if (updated_task_opt.has_value()) {
        Task updated_task = updated_task_opt.value();
        EXPECT_EQ(updated_task.status, "doing");
    }
}

// 测试用户2（非所有者）无法更新用户1的任务
TEST_F(PermissionTest, UserCannotUpdateOthersTask) {
    // 用户2尝试更新用户1的任务，应该返回空
    std::optional<Task> updated_task_opt = task_service_->UpdateTask(user2_id_, task_id_, std::nullopt, std::nullopt, std::nullopt, "doing", std::nullopt, std::nullopt, std::nullopt);
    
    // 验证更新失败
    EXPECT_FALSE(updated_task_opt.has_value());
}

// 测试用户1（所有者）可以删除自己的项目
TEST_F(PermissionTest, UserCanDeleteOwnProject) {
    // 用户1删除自己的项目
    project_service_->DeleteProject(user1_id_, project_id_);
    
    // 验证项目已删除
    EXPECT_THROW(
        project_service_->GetProjectById(user1_id_, project_id_),
        ProjectServiceException
    );
    
    // 重置项目ID，避免TearDown时重复删除
    project_id_ = 0;
}

// 测试用户2（非所有者）无法删除用户1的项目
TEST_F(PermissionTest, UserCannotDeleteOthersProject) {
    // 用户2尝试删除用户1的项目，应该抛出权限异常
    EXPECT_THROW(
        project_service_->DeleteProject(user2_id_, project_id_),
        ProjectServiceException
    );
}

