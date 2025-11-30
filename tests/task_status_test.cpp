// 任务状态流转测试

#include <gtest/gtest.h>

#include "task_service.h"
#include "database.h"
#include "auth_service.h"
#include "config.h"

class TaskStatusTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化数据库
        database_ = std::make_shared<Database>(config::kDatabasePath, config::kDatabaseConnectionPoolSize);
        
        // 初始化认证服务
        auth_service_ = std::make_shared<AuthService>(database_);
        
        // 初始化任务服务
        task_service_ = std::make_shared<TaskService>(database_, auth_service_);
        
        // 创建测试用户（如果不存在）
        auto existing_user = database_->GetUserByEmail("test_status@example.com");
        if (existing_user) {
            test_user_id_ = existing_user->id;
        } else {
            User user;
            user.name = "Test User";
            user.email = "test_status@example.com";
            user.password_hash = "test_hash";
            
            if (database_->CreateUser(user)) {
                auto new_user = database_->GetUserByEmail("test_status@example.com");
                if (new_user) {
                    test_user_id_ = new_user->id;
                }
            }
        }
        
        // 创建测试项目
        Project project;
        project.name = "Test Project";
        project.description = "Test Project Description";
        project.owner_user_id = test_user_id_;
        
        test_project_id_ = database_->CreateProject(project);
        
        // 创建测试任务
        Task task;
        task.title = "Test Task";
        task.description = "Test Task Description";
        task.status = "todo";
        task.priority = "medium";
        task.project_id = test_project_id_;
        task.assignee_user_id = test_user_id_;
        
        test_task_id_ = database_->CreateTask(task);
    }
    
    void TearDown() override {
        // 删除测试任务
        if (test_task_id_ > 0) {
            database_->DeleteTask(test_task_id_);
        }
        
        // 删除测试项目
        if (test_project_id_ > 0) {
            database_->DeleteProject(test_project_id_);
        }
        
        // 删除测试用户
        if (test_user_id_ > 0) {
            database_->DeleteUser(test_user_id_);
        }
    }
    
    std::shared_ptr<Database> database_;
    std::shared_ptr<AuthService> auth_service_;
    std::shared_ptr<TaskService> task_service_;
    
    int test_user_id_ = 0;
    int test_project_id_ = 0;
    int test_task_id_ = 0;
};

// 测试合法的状态流转：todo -> doing
TEST_F(TaskStatusTest, ValidStatusTransitionTodoToDoing) {
    // 获取当前任务
    auto task = database_->GetTaskById(test_task_id_);
    ASSERT_TRUE(task);
    EXPECT_EQ(task->status, "todo");
    
    // 更新任务状态为doing
    std::optional<std::string> status = "doing";
    
    auto updated_task = task_service_->UpdateTask(test_user_id_, test_task_id_, std::nullopt, std::nullopt, std::nullopt, status);
    ASSERT_TRUE(updated_task);
    
    // 验证任务状态已更新为doing
    EXPECT_EQ(updated_task->status, "doing");
    
    // 从数据库中重新获取任务验证
    auto refreshed_task = database_->GetTaskById(test_task_id_);
    ASSERT_TRUE(refreshed_task);
    EXPECT_EQ(refreshed_task->status, "doing");
}

// 测试合法的状态流转：doing -> done
TEST_F(TaskStatusTest, ValidStatusTransitionDoingToDone) {
    // 首先将任务状态更新为doing
    std::optional<std::string> status = "doing";
    task_service_->UpdateTask(test_user_id_, test_task_id_, std::nullopt, std::nullopt, std::nullopt, status);
    
    // 获取当前任务
    auto task = database_->GetTaskById(test_task_id_);
    ASSERT_TRUE(task);
    EXPECT_EQ(task->status, "doing");
    
    // 更新任务状态为done
    status = "done";
    auto updated_task = task_service_->UpdateTask(test_user_id_, test_task_id_, std::nullopt, std::nullopt, std::nullopt, status);
    ASSERT_TRUE(updated_task);
    
    // 验证任务状态已更新为done
    EXPECT_EQ(updated_task->status, "done");
    
    // 从数据库中重新获取任务验证
    auto refreshed_task = database_->GetTaskById(test_task_id_);
    ASSERT_TRUE(refreshed_task);
    EXPECT_EQ(refreshed_task->status, "done");
}

// 测试非法的状态流转：done -> todo
TEST_F(TaskStatusTest, InvalidStatusTransitionDoneToTodo) {
    // 首先将任务状态更新为done
    std::optional<std::string> status = "doing";
    task_service_->UpdateTask(test_user_id_, test_task_id_, std::nullopt, std::nullopt, std::nullopt, status);
    
    status = "done";
    task_service_->UpdateTask(test_user_id_, test_task_id_, std::nullopt, std::nullopt, std::nullopt, status);
    
    // 获取当前任务
    auto task = database_->GetTaskById(test_task_id_);
    ASSERT_TRUE(task);
    EXPECT_EQ(task->status, "done");
    
    // 尝试将任务状态更新为todo（非法流转）
    status = "todo";
    
    // 验证是否抛出异常
    EXPECT_THROW(task_service_->UpdateTask(test_user_id_, test_task_id_, std::nullopt, std::nullopt, std::nullopt, status), TaskServiceException);
    
    // 验证任务状态仍为done
    auto refreshed_task = database_->GetTaskById(test_task_id_);
    ASSERT_TRUE(refreshed_task);
    EXPECT_EQ(refreshed_task->status, "done");
}

// 测试非法的状态流转：todo -> done
TEST_F(TaskStatusTest, InvalidStatusTransitionTodoToDone) {
    // 获取当前任务
    auto task = database_->GetTaskById(test_task_id_);
    ASSERT_TRUE(task);
    EXPECT_EQ(task->status, "todo");
    
    // 尝试将任务状态直接更新为done（非法流转）
    std::optional<std::string> status = "done";
    
    // 验证是否抛出异常
    EXPECT_THROW(task_service_->UpdateTask(test_user_id_, test_task_id_, std::nullopt, std::nullopt, std::nullopt, status), TaskServiceException);
    
    // 验证任务状态仍为todo
    auto refreshed_task = database_->GetTaskById(test_task_id_);
    ASSERT_TRUE(refreshed_task);
    EXPECT_EQ(refreshed_task->status, "todo");
}

