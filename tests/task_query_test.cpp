// 任务查询过滤测试

#include <gtest/gtest.h>

#include "task_service.h"
#include "database.h"
#include "config.h"
#include "utils.h"

class TaskQueryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化数据库
        database_ = std::make_shared<Database>(config::kDatabasePath, config::kDatabaseConnectionPoolSize);
        
        // 初始化认证服务
        auth_service_ = std::make_shared<AuthService>(database_);
        
        // 初始化任务服务
        task_service_ = std::make_shared<TaskService>(database_, auth_service_);
        
        // 创建测试用户（如果不存在）
        auto existing_user = database_->GetUserByEmail("test_query@example.com");
        if (existing_user) {
            test_user_id_ = existing_user->id;
        } else {
            User user;
            user.name = "Test User";
            user.email = "test_query@example.com";
            user.password_hash = "test_hash";
            
            if (database_->CreateUser(user)) {
                auto new_user = database_->GetUserByEmail("test_query@example.com");
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
        
        // 创建测试任务1：todo状态，高优先级，无截止日期
        Task task1;
        task1.title = "Test Task 1";
        task1.description = "Test Task 1 Description";
        task1.status = "todo";
        task1.priority = "high";
        task1.project_id = test_project_id_;
        task1.assignee_user_id = test_user_id_;
        
        test_task_id_1_ = database_->CreateTask(task1);
        
        // 创建测试任务2：doing状态，中优先级，明天截止
        Task task2;
        task2.title = "Test Task 2";
        task2.description = "Test Task 2 Description";
        task2.status = "doing";
        task2.priority = "medium";
        task2.project_id = test_project_id_;
        task2.assignee_user_id = test_user_id_;
        
        // 设置明天的截止日期
        auto tomorrow = time_utils::Now() + std::chrono::hours(24);
        task2.due_date = tomorrow;
        
        test_task_id_2_ = database_->CreateTask(task2);
        
        // 创建测试任务3：done状态，低优先级，昨天截止
        Task task3;
        task3.title = "Test Task 3";
        task3.description = "Test Task 3 Description";
        task3.status = "done";
        task3.priority = "low";
        task3.project_id = test_project_id_;
        task3.assignee_user_id = test_user_id_;
        
        // 设置昨天的截止日期
        auto yesterday = time_utils::Now() - std::chrono::hours(24);
        task3.due_date = yesterday;
        
        test_task_id_3_ = database_->CreateTask(task3);
        
        // 创建测试任务4：todo状态，中优先级，包含关键字"important"
        Task task4;
        task4.title = "Important Test Task 4";
        task4.description = "This is an important task";
        task4.status = "todo";
        task4.priority = "medium";
        task4.project_id = test_project_id_;
        task4.assignee_user_id = test_user_id_;
        
        test_task_id_4_ = database_->CreateTask(task4);
    }
    
    void TearDown() override {
        // 删除测试任务
        if (test_task_id_1_ > 0) {
            database_->DeleteTask(test_task_id_1_);
        }
        
        if (test_task_id_2_ > 0) {
            database_->DeleteTask(test_task_id_2_);
        }
        
        if (test_task_id_3_ > 0) {
            database_->DeleteTask(test_task_id_3_);
        }
        
        if (test_task_id_4_ > 0) {
            database_->DeleteTask(test_task_id_4_);
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
    
    int test_task_id_1_ = 0;
    int test_task_id_2_ = 0;
    int test_task_id_3_ = 0;
    int test_task_id_4_ = 0;
};

// 测试按状态过滤任务
TEST_F(TaskQueryTest, QueryTasksByStatus) {
    // 查询todo状态的任务
    TaskQueryParams params;
    params.status = "todo";
    
    std::vector<Task> tasks = task_service_->SearchTasks(test_user_id_, params);
    
    // 验证查询结果是否包含任务1和任务4
    EXPECT_EQ(tasks.size(), 2);
    
    bool found_task1 = false;
    bool found_task4 = false;
    
    for (const auto& task : tasks) {
        if (task.id == test_task_id_1_) {
            found_task1 = true;
        } else if (task.id == test_task_id_4_) {
            found_task4 = true;
        }
    }
    
    EXPECT_TRUE(found_task1);
    EXPECT_TRUE(found_task4);
}

// 测试按优先级过滤任务
TEST_F(TaskQueryTest, QueryTasksByPriority) {
    // 查询高优先级的任务
    TaskQueryParams params;
    params.priority = "high";
    
    std::vector<Task> tasks = task_service_->SearchTasks(test_user_id_, params);
    
    // 验证查询结果是否只包含任务1
    EXPECT_EQ(tasks.size(), 1);
    EXPECT_EQ(tasks[0].id, test_task_id_1_);
}

// 测试按截止日期过滤任务（截止日期之前）
TEST_F(TaskQueryTest, QueryTasksByDueBefore) {
    // 查询今天之前截止的任务
    auto today = time_utils::Now();
    std::string today_str = time_utils::ToIsoString(today);
    
    TaskQueryParams params;
    params.due_before = today;
    
    std::vector<Task> tasks = task_service_->SearchTasks(test_user_id_, params);
    
    // 验证查询结果是否只包含任务3（昨天截止）
    EXPECT_EQ(tasks.size(), 1);
    EXPECT_EQ(tasks[0].id, test_task_id_3_);
}

// 测试按截止日期过滤任务（截止日期之后）
TEST_F(TaskQueryTest, QueryTasksByDueAfter) {
    // 查询今天之后截止的任务
    auto today = time_utils::Now();
    std::string today_str = time_utils::ToIsoString(today);
    
    TaskQueryParams params;
    params.due_after = today;
    
    std::vector<Task> tasks = task_service_->SearchTasks(test_user_id_, params);
    
    // 验证查询结果是否只包含任务2（明天截止）
    EXPECT_EQ(tasks.size(), 1);
    EXPECT_EQ(tasks[0].id, test_task_id_2_);
}

// 测试按关键字过滤任务
TEST_F(TaskQueryTest, QueryTasksByKeyword) {
    // 查询包含关键字"important"的任务
    TaskQueryParams params;
    params.keyword = "important";
    
    std::vector<Task> tasks = task_service_->SearchTasks(test_user_id_, params);
    
    // 验证查询结果是否只包含任务4
    EXPECT_EQ(tasks.size(), 1);
    EXPECT_EQ(tasks[0].id, test_task_id_4_);
}

// 测试组合过滤条件
TEST_F(TaskQueryTest, QueryTasksWithCombinedFilters) {
    // 查询todo状态且中优先级的任务
    TaskQueryParams params;
    params.status = "todo";
    params.priority = "medium";
    
    std::vector<Task> tasks = task_service_->SearchTasks(test_user_id_, params);
    
    // 验证查询结果是否只包含任务4
    EXPECT_EQ(tasks.size(), 1);
    EXPECT_EQ(tasks[0].id, test_task_id_4_);
}

