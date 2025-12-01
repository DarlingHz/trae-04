#ifndef WATCH_RECORD_REPOSITORY_H
#define WATCH_RECORD_REPOSITORY_H

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <chrono>
#include "../model/WatchRecord.h"

namespace repository {

class WatchRecordRepository {
public:
    WatchRecordRepository() = default;
    ~WatchRecordRepository() = default;

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
    bool updateWatchRecord(const std::shared_ptr<model::WatchRecord>& record);

    // 删除观影记录
    bool deleteWatchRecord(int id);

    // 查询用户的所有观影记录（用于统计）
    std::vector<std::shared_ptr<model::WatchRecord>> getAllWatchRecordsByUserId(int user_id);

private:
    WatchRecordRepository(const WatchRecordRepository&) = delete;
    WatchRecordRepository& operator=(const WatchRecordRepository&) = delete;
};

} // namespace repository

#endif // WATCH_RECORD_REPOSITORY_H
