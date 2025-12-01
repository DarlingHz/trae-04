#ifndef MOVIE_H
#define MOVIE_H

#include <string>
#include <chrono>

namespace model {

class Movie {
public:
    Movie() = default;
    Movie(int id, std::string title, std::string type, int duration, int status, std::chrono::system_clock::time_point created_at, std::chrono::system_clock::time_point updated_at)
        : id_(id), title_(std::move(title)), type_(std::move(type)), duration_(duration), status_(status), created_at_(created_at), updated_at_(updated_at) {}

    int getId() const { return id_; }
    void setId(int id) { id_ = id; }

    const std::string& getTitle() const { return title_; }
    void setTitle(std::string title) { title_ = std::move(title); }

    const std::string& getType() const { return type_; }
    void setType(std::string type) { type_ = std::move(type); }

    int getDuration() const { return duration_; }
    void setDuration(int duration) { duration_ = duration; }

    int getStatus() const { return status_; }
    void setStatus(int status) { status_ = status; }

    const std::chrono::system_clock::time_point& getCreatedAt() const { return created_at_; }
    void setCreatedAt(std::chrono::system_clock::time_point created_at) { created_at_ = created_at; }

    const std::chrono::system_clock::time_point& getUpdatedAt() const { return updated_at_; }
    void setUpdatedAt(std::chrono::system_clock::time_point updated_at) { updated_at_ = updated_at; }

private:
    int id_ = 0;
    std::string title_;
    std::string type_;
    int duration_ = 0;
    int status_ = 1;
    std::chrono::system_clock::time_point created_at_;
    std::chrono::system_clock::time_point updated_at_;
};

} // namespace model

#endif // MOVIE_H
