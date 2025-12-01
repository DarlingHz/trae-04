#include "MovieService.h"
#include "../utils/Logger.h"
#include <string>

namespace service {

std::shared_ptr<model::Movie> MovieService::createMovie(const std::string& title, const std::string& type, int duration) {
    // 验证输入
    if (title.empty()) {
        LOG_ERROR("Movie title cannot be empty");
        return nullptr;
    }

    if (title.length() > 100) {
        LOG_ERROR("Movie title is too long (max 100 characters)");
        return nullptr;
    }

    if (type.empty()) {
        LOG_ERROR("Movie type cannot be empty");
        return nullptr;
    }

    if (type.length() > 50) {
        LOG_ERROR("Movie type is too long (max 50 characters)");
        return nullptr;
    }

    if (duration <= 0) {
        LOG_ERROR("Invalid movie duration: " + std::to_string(duration));
        return nullptr;
    }

    // 创建影片
    auto movie = movie_repository_->createMovie(title, type, duration);
    if (movie) {
        LOG_INFO("Movie created successfully: " + std::to_string(movie->getId()) + " - " + movie->getTitle());
    } else {
        LOG_ERROR("Failed to create movie: " + title);
    }

    return movie;
}

std::shared_ptr<model::Movie> MovieService::getMovieById(int id) {
    if (id <= 0) {
        LOG_ERROR("Invalid movie ID: " + std::to_string(id));
        return nullptr;
    }

    auto movie = movie_repository_->getMovieById(id);
    if (movie) {
        if (movie->getStatus() == 1) {
            LOG_DEBUG("Movie retrieved successfully: " + std::to_string(movie->getId()) + " - " + movie->getTitle());
        } else {
            LOG_DEBUG("Movie is deleted: " + std::to_string(movie->getId()) + " - " + movie->getTitle());
            movie.reset();
        }
    } else {
        LOG_DEBUG("Movie not found with ID: " + std::to_string(id));
    }

    return movie;
}

std::vector<std::shared_ptr<model::Movie>> MovieService::getMovies(const std::string& keyword, const std::string& type, int page, int page_size) {
    // 验证分页参数
    if (page <= 0) {
        LOG_WARN("Invalid page number, using default: 1");
        page = 1;
    }

    if (page_size <= 0 || page_size > 100) {
        LOG_WARN("Invalid page size, using default: 10");
        page_size = 10;
    }

    auto movies = movie_repository_->getMovies(keyword, type, page, page_size);
    LOG_DEBUG("Retrieved " + std::to_string(movies.size()) + " movies for page " + std::to_string(page));
    return movies;
}

bool MovieService::updateMovie(int id, const std::string& title, const std::string& type, int duration) {
    if (id <= 0) {
        LOG_ERROR("Invalid movie ID: " + std::to_string(id));
        return false;
    }

    // 验证输入
    if (title.empty()) {
        LOG_ERROR("Movie title cannot be empty");
        return false;
    }

    if (title.length() > 100) {
        LOG_ERROR("Movie title is too long (max 100 characters)");
        return false;
    }

    if (type.empty()) {
        LOG_ERROR("Movie type cannot be empty");
        return false;
    }

    if (type.length() > 50) {
        LOG_ERROR("Movie type is too long (max 50 characters)");
        return false;
    }

    if (duration <= 0) {
        LOG_ERROR("Invalid movie duration: " + std::to_string(duration));
        return false;
    }

    // 检查影片是否存在且未被删除
    auto movie = movie_repository_->getMovieById(id);
    if (!movie) {
        LOG_ERROR("Movie not found with ID: " + std::to_string(id));
        return false;
    }

    if (movie->getStatus() != 1) {
        LOG_ERROR("Cannot update deleted movie: " + std::to_string(id));
        return false;
    }

    // 更新影片信息
    movie->setTitle(title);
    movie->setType(type);
    movie->setDuration(duration);
    bool success = movie_repository_->updateMovie(movie);
    if (success) {
        LOG_INFO("Movie updated successfully: " + std::to_string(id) + " - " + title);
    } else {
        LOG_ERROR("Failed to update movie with ID: " + std::to_string(id));
    }

    return success;
}

bool MovieService::deleteMovie(int id) {
    if (id <= 0) {
        LOG_ERROR("Invalid movie ID: " + std::to_string(id));
        return false;
    }

    // 检查影片是否存在
    auto movie = movie_repository_->getMovieById(id);
    if (!movie) {
        LOG_ERROR("Movie not found with ID: " + std::to_string(id));
        return false;
    }

    if (movie->getStatus() != 1) {
        LOG_WARN("Movie is already deleted: " + std::to_string(id));
        return true;
    }

    // 逻辑删除影片
    bool success = movie_repository_->deleteMovie(id);
    if (success) {
        LOG_INFO("Movie deleted successfully: " + std::to_string(id) + " - " + movie->getTitle());
    } else {
        LOG_ERROR("Failed to delete movie with ID: " + std::to_string(id));
    }

    return success;
}

std::vector<std::shared_ptr<model::Movie>> MovieService::getAllActiveMovies() {
    auto movies = movie_repository_->getAllActiveMovies();
    LOG_DEBUG("Retrieved " + std::to_string(movies.size()) + " active movies");
    return movies;
}

} // namespace service
