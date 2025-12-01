#ifndef WATCH_RECORD_H
#define WATCH_RECORD_H

#include <string>
#include <chrono>

namespace model {

class WatchRecord {
public:
    WatchRecord() = default;
    WatchRecord(int id, int user_id, int movie_id, std::chrono::system_clock::time_point start_time, int watch_duration, int is_finished, int rating, std::string comment, std::chrono::system_clock::time_point created_at, std::chrono::system_clock::time_point updated_at)
        : id_(id), user_id_(user_id), movie_id_(movie_id), start_time_(start_time), watch_duration_(watch_duration), is_finished_(is_finished), rating_(rating), comment_(std::move(comment)), created_at_(created_at), updated_at_(updated_at) {}

    int getId() const { return id_; }
    void setId(int id) { id_ = id; }

    int getUserId() const { return user_id_; }
    void setUserId(int user_id) { user_id_ = user_id; }

    int getMovieId() const { return movie_id_; }
    void setMovieId(int movie_id) { movie_id_ = movie_id; }

    const std::chrono::system_clock::time_point& getStartTime() const { return start_time_; }
    void setStartTime(std::chrono::system_clock::time_point start_time) { start_time_ = start_time; }

    int getWatchDuration() const { return watch_duration_; }
    void setWatchDuration(int watch_duration) { watch_duration_ = watch_duration; }

    int getIsFinished() const { return is_finished_; }
    void setIsFinished(int is_finished) { is_finished_ = is_finished; }

    int getRating() const { return rating_; }
    void setRating(int rating) { rating_ = rating; }

    const std::string& getComment() const { return comment_; }
    void setComment(std::string comment) { comment_ = std::move(comment); }

    const std::chrono::system_clock::time_point& getCreatedAt() const { return created_at_; }
    void setCreatedAt(std::chrono::system_clock::time_point created_at) { created_at_ = created_at; }

    const std::chrono::system_clock::time_point& getUpdatedAt() const { return updated_at_; }
    void setUpdatedAt(std::chrono::system_clock::time_point updated_at) { updated_at_ = updated_at; }

private:
    int id_ = 0;
    int user_id_ = 0;
    int movie_id_ = 0;
    std::chrono::system_clock::time_point start_time_;
    int watch_duration_ = 0;
    int is_finished_ = 0;
    int rating_ = 0;
    std::string comment_;
    std::chrono::system_clock::time_point created_at_;
    std::chrono::system_clock::time_point updated_at_;
};

} // namespace model

#endif // WATCH_RECORD_H
