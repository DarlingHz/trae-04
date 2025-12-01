#ifndef MOVIE_SERVICE_H
#define MOVIE_SERVICE_H

#include <memory>
#include <vector>
#include <string>
#include "../model/Movie.h"
#include "../repository/MovieRepository.h"

namespace service {

class MovieService {
public:
    MovieService() : movie_repository_(std::make_shared<repository::MovieRepository>()) {}
    explicit MovieService(std::shared_ptr<repository::MovieRepository> movie_repository)
        : movie_repository_(std::move(movie_repository)) {}
    ~MovieService() = default;

    // 创建影片
    std::shared_ptr<model::Movie> createMovie(const std::string& title, const std::string& type, int duration);

    // 根据 ID 查询影片
    std::shared_ptr<model::Movie> getMovieById(int id);

    // 分页查询影片列表
    std::vector<std::shared_ptr<model::Movie>> getMovies(const std::string& keyword = "", const std::string& type = "", int page = 1, int page_size = 10);

    // 更新影片
    bool updateMovie(int id, const std::string& title, const std::string& type, int duration);

    // 逻辑删除影片
    bool deleteMovie(int id);

    // 查询所有未删除的影片
    std::vector<std::shared_ptr<model::Movie>> getAllActiveMovies();

private:
    std::shared_ptr<repository::MovieRepository> movie_repository_;

    MovieService(const MovieService&) = delete;
    MovieService& operator=(const MovieService&) = delete;
};

} // namespace service

#endif // MOVIE_SERVICE_H
