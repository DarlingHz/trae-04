#include "WatchRecordService.h"
#include "../utils/Logger.h"
#include <string>
#include <optional>
#include <chrono>

namespace service {

std::shared_ptr<model::WatchRecord> WatchRecordService::createWatchRecord(
    int user_id, int movie_id, const std::chrono::system_clock::time_point& start_time,
    int watch_duration, bool is_finished, std::optional<int> rating, std::optional<std::string> comment) {

    // 验证输入
    if (user_id <= 0) {
        LOG_ERROR("Invalid user ID: " + std::to_string(user_id));
        return nullptr;
    }

    if (movie_id <= 0) {
        LOG_ERROR("Invalid movie ID: " + std::to_string(movie_id));
        return nullptr;
    }

    if (watch_duration < 0) {
        LOG_ERROR("Invalid watch duration: " + std::to_string(watch_duration));
        return nullptr;
    }

    // 验证评分
    if (rating && (*rating < 0 || *rating > 5)) {
        LOG_ERROR("Invalid rating: " + std::to_string(*rating) + " (must be between 0 and 5)");
        return nullptr;
    }

    // 验证评论长度
    if (comment && comment->length() > 500) {
        LOG_ERROR("Comment is too long (max 500 characters)");
        return nullptr;
    }

    // 检查用户是否存在
    auto user = user_repository_->getUserById(user_id);
    if (!user) {
        LOG_ERROR("User not found with ID: " + std::to_string(user_id));
        return nullptr;
    }

    // 检查影片是否存在且未被删除
    auto movie = movie_repository_->getMovieById(movie_id);
    if (!movie) {
        LOG_ERROR("Movie not found with ID: " + std::to_string(movie_id));
        return nullptr;
    }

    if (movie->getStatus() != 1) {
        LOG_ERROR("Cannot create watch record for deleted movie: " + std::to_string(movie_id));
        return nullptr;
    }

    // 创建观影记录
    auto record = watch_record_repository_->createWatchRecord(
        user_id, movie_id, start_time, watch_duration, is_finished,
        rating, comment);
    
    if (record) {
        LOG_INFO("Watch record created successfully: User " + std::to_string(user_id) + ", Movie " + std::to_string(movie_id));
    } else {
        LOG_ERROR("Failed to create watch record: User " + std::to_string(user_id) + ", Movie " + std::to_string(movie_id));
    }

    return record;
}

std::shared_ptr<model::WatchRecord> WatchRecordService::getWatchRecordById(int id) {
    if (id <= 0) {
        LOG_ERROR("Invalid watch record ID: " + std::to_string(id));
        return nullptr;
    }

    auto record = watch_record_repository_->getWatchRecordById(id);
    if (record) {
        LOG_DEBUG("Watch record retrieved successfully: " + std::to_string(record->getId()));
    } else {
        LOG_DEBUG("Watch record not found with ID: " + std::to_string(id));
    }

    return record;
}

std::vector<std::shared_ptr<model::WatchRecord>> WatchRecordService::getWatchRecordsByUserId(
    int user_id, const std::optional<std::chrono::system_clock::time_point>& start_time,
    const std::optional<std::chrono::system_clock::time_point>& end_time, int page, int page_size) {

    if (user_id <= 0) {
        LOG_ERROR("Invalid user ID: " + std::to_string(user_id));
        return {};
    }

    // 验证分页参数
    if (page <= 0) {
        LOG_WARN("Invalid page number, using default: 1");
        page = 1;
    }

    if (page_size <= 0 || page_size > 100) {
        LOG_WARN("Invalid page size, using default: 10");
        page_size = 10;
    }

    // 检查用户是否存在
    auto user = user_repository_->getUserById(user_id);
    if (!user) {
        LOG_ERROR("User not found with ID: " + std::to_string(user_id));
        return {};
    }

    auto records = watch_record_repository_->getWatchRecordsByUserId(user_id, start_time, end_time, page, page_size);
    LOG_DEBUG("Retrieved " + std::to_string(records.size()) + " watch records for user " + std::to_string(user_id) + ", page " + std::to_string(page));
    return records;
}

bool WatchRecordService::updateWatchRecord(
    int id, const std::chrono::system_clock::time_point& start_time,
    int watch_duration, bool is_finished, std::optional<int> rating, std::optional<std::string> comment) {

    if (id <= 0) {
        LOG_ERROR("Invalid watch record ID: " + std::to_string(id));
        return false;
    }

    // 验证输入
    if (watch_duration < 0) {
        LOG_ERROR("Invalid watch duration: " + std::to_string(watch_duration));
        return false;
    }

    if (rating && (*rating < 1 || *rating > 5)) {
        LOG_ERROR("Invalid rating: " + std::to_string(*rating) + " (must be between 1 and 5)");
        return false;
    }

    if (comment && comment->length() > 500) {
        LOG_ERROR("Comment is too long (max 500 characters)");
        return false;
    }

    // 检查观影记录是否存在
    auto record = watch_record_repository_->getWatchRecordById(id);
    if (!record) {
        LOG_ERROR("Watch record not found with ID: " + std::to_string(id));
        return false;
    }

    // 检查用户是否存在
    auto user = user_repository_->getUserById(record->getUserId());
    if (!user) {
        LOG_ERROR("User not found for watch record: " + std::to_string(id));
        return false;
    }

    // 检查影片是否存在且未被删除
    auto movie = movie_repository_->getMovieById(record->getMovieId());
    if (!movie) {
        LOG_ERROR("Movie not found for watch record: " + std::to_string(id));
        return false;
    }

    if (movie->getStatus() != 1) {
        LOG_ERROR("Cannot update watch record for deleted movie: " + std::to_string(movie->getId()));
        return false;
    }

    // 更新观影记录
    record->setStartTime(start_time);
    record->setWatchDuration(watch_duration);
    record->setIsFinished(is_finished ? 1 : 0);
    record->setRating(rating ? *rating : 0);
    record->setComment(comment ? *comment : "");
    
    bool success = watch_record_repository_->updateWatchRecord(record);
    if (success) {
        LOG_INFO("Watch record updated successfully: " + std::to_string(id));
    } else {
        LOG_ERROR("Failed to update watch record with ID: " + std::to_string(id));
    }

    return success;
}

bool WatchRecordService::deleteWatchRecord(int id) {
    if (id <= 0) {
        LOG_ERROR("Invalid watch record ID: " + std::to_string(id));
        return false;
    }

    // 检查观影记录是否存在
    auto record = watch_record_repository_->getWatchRecordById(id);
    if (!record) {
        LOG_ERROR("Watch record not found with ID: " + std::to_string(id));
        return false;
    }

    // 删除观影记录
    bool success = watch_record_repository_->deleteWatchRecord(id);
    if (success) {
        LOG_INFO("Watch record deleted successfully: " + std::to_string(id));
    } else {
        LOG_ERROR("Failed to delete watch record with ID: " + std::to_string(id));
    }

    return success;
}

std::vector<std::shared_ptr<model::WatchRecord>> WatchRecordService::getAllWatchRecordsByUserId(int user_id) {
    if (user_id <= 0) {
        LOG_ERROR("Invalid user ID: " + std::to_string(user_id));
        return {};
    }

    // 检查用户是否存在
    auto user = user_repository_->getUserById(user_id);
    if (!user) {
        LOG_ERROR("User not found with ID: " + std::to_string(user_id));
        return {};
    }

    auto records = watch_record_repository_->getAllWatchRecordsByUserId(user_id);
    LOG_DEBUG("Retrieved " + std::to_string(records.size()) + " watch records for user " + std::to_string(user_id));
    return records;
}

} // namespace service
