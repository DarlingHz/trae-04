#include "MovieRepository.h"
#include "../utils/DbConnectionPool.h"
#include "../utils/Logger.h"
#include "../utils/TimeUtils.h"
#include <sqlite3.h>
#include <string>
#include <sstream>

namespace repository {

std::shared_ptr<model::Movie> MovieRepository::createMovie(const std::string& title, const std::string& type, int duration) {
    auto conn = utils::g_db_pool.getConnection();
    if (!conn) {
        LOG_ERROR("Failed to get database connection");
        return nullptr;
    }

    const char* sql = "INSERT INTO movies (title, type, duration) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    rc = sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 1: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    rc = sqlite3_bind_text(stmt, 2, type.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 2: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    rc = sqlite3_bind_int(stmt, 3, duration);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 3: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    int movie_id = sqlite3_last_insert_rowid(conn.get());
    sqlite3_finalize(stmt);
    utils::g_db_pool.releaseConnection(conn);

    return getMovieById(movie_id);
}

std::shared_ptr<model::Movie> MovieRepository::getMovieById(int id) {
    auto conn = utils::g_db_pool.getConnection();
    if (!conn) {
        LOG_ERROR("Failed to get database connection");
        return nullptr;
    }

    const char* sql = "SELECT id, title, type, duration, status, created_at, updated_at FROM movies WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    rc = sqlite3_bind_int(stmt, 1, id);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    std::shared_ptr<model::Movie> movie = nullptr;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        int movie_id = sqlite3_column_int(stmt, 0);
        const char* title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        int duration = sqlite3_column_int(stmt, 3);
        int status = sqlite3_column_int(stmt, 4);
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

        // 转换时间字符串为 time_point
        auto created_at = utils::TimeUtils::isoStringToTimePoint(created_at_str);
        auto updated_at = utils::TimeUtils::isoStringToTimePoint(updated_at_str);

        movie = std::make_shared<model::Movie>(movie_id, title, type, duration, status, created_at, updated_at);
    } else if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
    }

    sqlite3_finalize(stmt);
    utils::g_db_pool.releaseConnection(conn);

    return movie;
}

std::vector<std::shared_ptr<model::Movie>> MovieRepository::getMovies(const std::string& keyword, const std::string& type, int page, int page_size) {
    auto conn = utils::g_db_pool.getConnection();
    if (!conn) {
        LOG_ERROR("Failed to get database connection");
        return {};
    }

    std::ostringstream sql; sql << "SELECT id, title, type, duration, status, created_at, updated_at FROM movies WHERE status = 1";
    if (!keyword.empty()) {
        sql << " AND (title LIKE ? OR type LIKE ?)";
    }
    if (!type.empty()) {
        sql << " AND type LIKE ?";
    }
    sql << " ORDER BY created_at DESC LIMIT ? OFFSET ?";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn.get(), sql.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return {};
    }

    int param_index = 1;
    if (!keyword.empty()) {
        std::string like_pattern = "%" + keyword + "%";
        rc = sqlite3_bind_text(stmt, param_index++, like_pattern.c_str(), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) {
            LOG_ERROR("Failed to bind keyword parameter 1: " + std::string(sqlite3_errmsg(conn.get())));
            sqlite3_finalize(stmt);
            utils::g_db_pool.releaseConnection(conn);
            return {};
        }
        rc = sqlite3_bind_text(stmt, param_index++, like_pattern.c_str(), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) {
            LOG_ERROR("Failed to bind keyword parameter 2: " + std::string(sqlite3_errmsg(conn.get())));
            sqlite3_finalize(stmt);
            utils::g_db_pool.releaseConnection(conn);
            return {};
        }
    }
    if (!type.empty()) {
        std::string like_pattern = "%" + type + "%";
        rc = sqlite3_bind_text(stmt, param_index++, like_pattern.c_str(), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) {
            LOG_ERROR("Failed to bind type parameter: " + std::string(sqlite3_errmsg(conn.get())));
            sqlite3_finalize(stmt);
            utils::g_db_pool.releaseConnection(conn);
            return {};
        }
    }

    int offset = (page - 1) * page_size;
    rc = sqlite3_bind_int(stmt, param_index++, page_size);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind limit parameter: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return {};
    }
    rc = sqlite3_bind_int(stmt, param_index++, offset);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind offset parameter: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return {};
    }

    std::vector<std::shared_ptr<model::Movie>> movies;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int movie_id = sqlite3_column_int(stmt, 0);
        const char* title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        int duration = sqlite3_column_int(stmt, 3);
        int status = sqlite3_column_int(stmt, 4);
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

        // 转换时间字符串为 time_point
        auto created_at = utils::TimeUtils::isoStringToTimePoint(created_at_str);
        auto updated_at = utils::TimeUtils::isoStringToTimePoint(updated_at_str);

        auto movie = std::make_shared<model::Movie>(movie_id, title, type, duration, status, created_at, updated_at);
        movies.push_back(movie);
    }

    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
    }

    sqlite3_finalize(stmt);
    utils::g_db_pool.releaseConnection(conn);

    return movies;
}

bool MovieRepository::updateMovie(const std::shared_ptr<model::Movie>& movie) {
    if (!movie) {
        LOG_ERROR("Invalid movie pointer");
        return false;
    }

    auto conn = utils::g_db_pool.getConnection();
    if (!conn) {
        LOG_ERROR("Failed to get database connection");
        return false;
    }

    const char* sql = "UPDATE movies SET title = ?, type = ?, duration = ?, status = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    rc = sqlite3_bind_text(stmt, 1, movie->getTitle().c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 1: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    rc = sqlite3_bind_text(stmt, 2, movie->getType().c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 2: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    rc = sqlite3_bind_int(stmt, 3, movie->getDuration());
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 3: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    rc = sqlite3_bind_int(stmt, 4, movie->getStatus());
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 4: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    rc = sqlite3_bind_int(stmt, 5, movie->getId());
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 5: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    int changes = sqlite3_changes(conn.get());
    sqlite3_finalize(stmt);
    utils::g_db_pool.releaseConnection(conn);

    return changes > 0;
}

bool MovieRepository::deleteMovie(int id) {
    auto conn = utils::g_db_pool.getConnection();
    if (!conn) {
        LOG_ERROR("Failed to get database connection");
        return false;
    }

    const char* sql = "UPDATE movies SET status = 0, updated_at = CURRENT_TIMESTAMP WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    rc = sqlite3_bind_int(stmt, 1, id);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    int changes = sqlite3_changes(conn.get());
    sqlite3_finalize(stmt);
    utils::g_db_pool.releaseConnection(conn);

    return changes > 0;
}

std::vector<std::shared_ptr<model::Movie>> MovieRepository::getAllActiveMovies() {
    auto conn = utils::g_db_pool.getConnection();
    if (!conn) {
        LOG_ERROR("Failed to get database connection");
        return {};
    }

    const char* sql = "SELECT id, title, type, duration, status, created_at, updated_at FROM movies WHERE status = 1 ORDER BY created_at DESC";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return {};
    }

    std::vector<std::shared_ptr<model::Movie>> movies;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int movie_id = sqlite3_column_int(stmt, 0);
        const char* title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        int duration = sqlite3_column_int(stmt, 3);
        int status = sqlite3_column_int(stmt, 4);
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

        // 转换时间字符串为 time_point
        auto created_at = utils::TimeUtils::isoStringToTimePoint(created_at_str);
        auto updated_at = utils::TimeUtils::isoStringToTimePoint(updated_at_str);

        auto movie = std::make_shared<model::Movie>(movie_id, title, type, duration, status, created_at, updated_at);
        movies.push_back(movie);
    }

    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
    }

    sqlite3_finalize(stmt);
    utils::g_db_pool.releaseConnection(conn);

    return movies;
}

} // namespace repository
