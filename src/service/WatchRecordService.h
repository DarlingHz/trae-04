#ifndef WATCH_RECORD_SERVICE_H
#define WATCH_RECORD_SERVICE_H

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <chrono>
#include "../model/WatchRecord.h"
#include "../repository/WatchRecordRepository.h"
#include "../repository/UserRepository.h"
#include "../repository/MovieRepository.h"

namespace service {

class WatchRecordService {
public:
    WatchRecordService()
        : watch_record_repository_(std::make_shared<repository::WatchRecordRepository>())
        , user_repository_(std::make_shared<repository::UserRepository>())
        , movie_repository_(std::make_shared<repository::MovieRepository>()) {}
    
    WatchRecordService(
        std::shared_ptr<repository::WatchRecordRepository> watch_record_repository,
        std::shared_ptr<repository::UserRepository> user_repository,
        std::shared_ptr<repository::MovieRepository> movie_repository)
        : watch_record_repository_(std::move(watch_record_repository))
        , user_repository_(std::move(user_repository))
        , movie_repository_(std::move(movie_repository)) {}
    
    ~WatchRecordService() = default;

    // 创建观影记录
    std::shared_ptr<model::WatchRecord> createWatchRecord(
        int user_id, int movie_id, const std::chrono::system_clock::time_point& start_time,
        int watch_duration, bool is_finished, std::optional<int> rating = std::nullopt,
        std::optional<std::string> comment = std::nullopt);

    // 根据 ID 查询观影记录
    std::shared_ptr<model::WatchRecord> getWatchRecordById(int id);

    // 根据用户 ID 分页查询观影记录
    std::vector<std::shared_ptr<model::WatchRecord>> getWatchRecordsByUserId(
        int user_id, const std::optional<std::chrono::system_clock::time_point>& start_time = std::nullopt,
        const std::optional<std::chrono::system_clock::time_point>& end_time = std::nullopt,
        int page = 1, int page_size = 10);

    // 更新观影记录
    bool updateWatchRecord(
        int id, const std::chrono::system_clock::time_point& start_time,
        int watch_duration, bool is_finished, std::optional<int> rating = std::nullopt,
        std::optional<std::string> comment = std::nullopt);

    // 删除观影记录
    bool deleteWatchRecord(int id);

    // 查询用户的所有观影记录（用于统计）
    std::vector<std::shared_ptr<model::WatchRecord>> getAllWatchRecordsByUserId(int user_id);

private:
    std::shared_ptr<repository::WatchRecordRepository> watch_record_repository_;
    std::shared_ptr<repository::UserRepository> user_repository_;
    std::shared_ptr<repository::MovieRepository> movie_repository_;

    WatchRecordService(const WatchRecordService&) = delete;
    WatchRecordService& operator=(const WatchRecordService&) = delete;
};

} // namespace service

#endif // WATCH_RECORD_SERVICE_H
