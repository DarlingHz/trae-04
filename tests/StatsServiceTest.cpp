#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <chrono>
#include "service/StatsService.h"
#include "repository/WatchRecordRepository.h"
#include "repository/MovieRepository.h"
#include "model/WatchRecord.h"
#include "model/Movie.h"

// 模拟 WatchRecordRepository
class MockWatchRecordRepository : public repository::WatchRecordRepository {
public:
    std::vector<std::shared_ptr<model::WatchRecord>> getAllWatchRecordsByUserId(int user_id) override {
        // 为测试用户 ID 1 返回模拟数据
        if (user_id == 1) {
            auto now = std::chrono::system_clock::now();
            auto yesterday = now - std::chrono::days(1);
            auto last_week = now - std::chrono::days(7);
            auto last_month = now - std::chrono::days(30);
            
            std::vector<std::shared_ptr<model::WatchRecord>> records;
            
            // 最近 30 天内的记录
            records.push_back(std::make_shared<model::WatchRecord>(1, 1, yesterday, 148, true, 8.8, "Great movie!"));
            records.push_back(std::make_shared<model::WatchRecord>(1, 2, last_week, 142, true, 9.3, "One of the best movies ever!"));
            
            // 30 天前的记录
            records.push_back(std::make_shared<model::WatchRecord>(1, 3, last_month, 152, true, 9.0, "Heath Ledger was amazing!"));
            
            return records;
        }
        
        return {};
    }
};

// 模拟 MovieRepository
class MockMovieRepository : public repository::MovieRepository {
public:
    std::shared_ptr<model::Movie> getMovieById(int movie_id) override {
        switch (movie_id) {
            case 1:
                return std::make_shared<model::Movie>(1, "Inception", "Sci-Fi", 148, 1);
            case 2:
                return std::make_shared<model::Movie>(2, "The Shawshank Redemption", "Drama", 142, 1);
            case 3:
                return std::make_shared<model::Movie>(3, "The Dark Knight", "Action", 152, 1);
            default:
                return nullptr;
        }
    }
    
    std::vector<std::shared_ptr<model::Movie>> getAllActiveMovies() override {
        std::vector<std::shared_ptr<model::Movie>> movies;
        movies.push_back(std::make_shared<model::Movie>(4, "Pulp Fiction", "Crime", 154, 1));
        movies.push_back(std::make_shared<model::Movie>(5, "Forrest Gump", "Drama", 142, 1));
        movies.push_back(std::make_shared<model::Movie>(6, "The Matrix", "Sci-Fi", 136, 1));
        return movies;
    }
};

TEST(StatsServiceTest, CalculateUserStats) {
    // 创建模拟的仓库实例
    auto watch_record_repo = std::make_shared<MockWatchRecordRepository>();
    auto movie_repo = std::make_shared<MockMovieRepository>();
    
    // 创建 StatsService 实例
    service::StatsService stats_service(watch_record_repo, movie_repo);
    
    // 计算用户 ID 1 的统计信息
    auto stats = stats_service.getUserStats(1);
    
    // 验证统计结果
    ASSERT_TRUE(stats.has_value());
    EXPECT_EQ(stats->total_movies_watched, 3);
    EXPECT_EQ(stats->total_watch_duration, 148 + 142 + 152);
    EXPECT_EQ(stats->last_30_days_count, 2);
    EXPECT_EQ(stats->last_30_days_duration, 148 + 142);
    EXPECT_NEAR(stats->average_rating, (8.8 + 9.3 + 9.0) / 3, 0.01);
    
    // 验证 Top3 类型
    EXPECT_EQ(stats->top_types.size(), 3);
    EXPECT_TRUE(stats->top_types.contains("Sci-Fi"));
    EXPECT_TRUE(stats->top_types.contains("Drama"));
    EXPECT_TRUE(stats->top_types.contains("Action"));
}

TEST(StatsServiceTest, GetRecommendations) {
    // 创建模拟的仓库实例
    auto watch_record_repo = std::make_shared<MockWatchRecordRepository>();
    auto movie_repo = std::make_shared<MockMovieRepository>();
    
    // 创建 StatsService 实例
    service::StatsService stats_service(watch_record_repo, movie_repo);
    
    // 获取用户 ID 1 的推荐列表
    auto recommendations = stats_service.getRecommendations(1, 5);
    
    // 验证推荐结果
    ASSERT_FALSE(recommendations.empty());
    EXPECT_LE(recommendations.size(), 5);
    
    // 验证推荐的影片都是用户未看过的
    for (const auto& movie : recommendations) {
        EXPECT_TRUE(movie->getId() == 4 || movie->getId() == 5 || movie->getId() == 6);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
