#ifndef MOVIE_REPOSITORY_H
#define MOVIE_REPOSITORY_H

#include <memory>
#include <vector>
#include <string>
#include "../model/Movie.h"

namespace repository {

class MovieRepository {
public:
    MovieRepository() = default;
    ~MovieRepository() = default;

    // 创建影片
    std::shared_ptr<model::Movie> createMovie(const std::string& title, const std::string& type, int duration);

    // 根据 ID 查询影片
    std::shared_ptr<model::Movie> getMovieById(int id);

    // 分页查询影片列表
    std::vector<std::shared_ptr<model::Movie>> getMovies(const std::string& keyword = "", const std::string& type = "", int page = 1, int page_size = 10);

    // 更新影片
    bool updateMovie(const std::shared_ptr<model::Movie>& movie);

    // 逻辑删除影片
    bool deleteMovie(int id);

    // 查询所有未删除的影片
    std::vector<std::shared_ptr<model::Movie>> getAllActiveMovies();

private:
    MovieRepository(const MovieRepository&) = delete;
    MovieRepository& operator=(const MovieRepository&) = delete;
};

} // namespace repository

#endif // MOVIE_REPOSITORY_H
