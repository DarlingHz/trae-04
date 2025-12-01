#include "UserRepository.h"
#include "../utils/DbConnectionPool.h"
#include "../utils/Logger.h"
#include "../utils/TimeUtils.h"
#include <sqlite3.h>
#include <string>

namespace repository {

std::shared_ptr<model::User> UserRepository::createUser(const std::string& nickname) {
    auto conn = utils::g_db_pool.getConnection();
    if (!conn) {
        LOG_ERROR("Failed to get database connection");
        return nullptr;
    }

    const char* sql = "INSERT INTO users (nickname) VALUES (?)";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    rc = sqlite3_bind_text(stmt, 1, nickname.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter: " + std::string(sqlite3_errmsg(conn.get())));
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

    int user_id = sqlite3_last_insert_rowid(conn.get());
    sqlite3_finalize(stmt);
    utils::g_db_pool.releaseConnection(conn);

    return getUserById(user_id);
}

std::shared_ptr<model::User> UserRepository::getUserById(int id) {
    auto conn = utils::g_db_pool.getConnection();
    if (!conn) {
        LOG_ERROR("Failed to get database connection");
        return nullptr;
    }

    const char* sql = "SELECT id, nickname, created_at, updated_at FROM users WHERE id = ?";
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

    std::shared_ptr<model::User> user = nullptr;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        int user_id = sqlite3_column_int(stmt, 0);
        const char* nickname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        // 转换时间字符串为 time_point
        auto created_at = utils::TimeUtils::isoStringToTimePoint(created_at_str);
        auto updated_at = utils::TimeUtils::isoStringToTimePoint(updated_at_str);

        user = std::make_shared<model::User>(user_id, nickname, created_at, updated_at);
    } else if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
    }

    sqlite3_finalize(stmt);
    utils::g_db_pool.releaseConnection(conn);

    return user;
}

std::shared_ptr<model::User> UserRepository::getUserByNickname(const std::string& nickname) {
    auto conn = utils::g_db_pool.getConnection();
    if (!conn) {
        LOG_ERROR("Failed to get database connection");
        return nullptr;
    }

    const char* sql = "SELECT id, nickname, created_at, updated_at FROM users WHERE nickname = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    rc = sqlite3_bind_text(stmt, 1, nickname.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return nullptr;
    }

    std::shared_ptr<model::User> user = nullptr;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        int user_id = sqlite3_column_int(stmt, 0);
        const char* nickname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        // 转换时间字符串为 time_point
        auto created_at = utils::TimeUtils::isoStringToTimePoint(created_at_str);
        auto updated_at = utils::TimeUtils::isoStringToTimePoint(updated_at_str);

        user = std::make_shared<model::User>(user_id, nickname, created_at, updated_at);
    } else if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
    }

    sqlite3_finalize(stmt);
    utils::g_db_pool.releaseConnection(conn);

    return user;
}

std::vector<std::shared_ptr<model::User>> UserRepository::getAllUsers() {
    auto conn = utils::g_db_pool.getConnection();
    if (!conn) {
        LOG_ERROR("Failed to get database connection");
        return {};
    }

    const char* sql = "SELECT id, nickname, created_at, updated_at FROM users";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return {};
    }

    std::vector<std::shared_ptr<model::User>> users;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int user_id = sqlite3_column_int(stmt, 0);
        const char* nickname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        // 转换时间字符串为 time_point
        auto created_at = utils::TimeUtils::isoStringToTimePoint(created_at_str);
        auto updated_at = utils::TimeUtils::isoStringToTimePoint(updated_at_str);

        auto user = std::make_shared<model::User>(user_id, nickname, created_at, updated_at);
        users.push_back(user);
    }

    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
    }

    sqlite3_finalize(stmt);
    utils::g_db_pool.releaseConnection(conn);

    return users;
}

bool UserRepository::updateUser(const std::shared_ptr<model::User>& user) {
    if (!user) {
        LOG_ERROR("Invalid user pointer");
        return false;
    }

    auto conn = utils::g_db_pool.getConnection();
    if (!conn) {
        LOG_ERROR("Failed to get database connection");
        return false;
    }

    const char* sql = "UPDATE users SET nickname = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    rc = sqlite3_bind_text(stmt, 1, user->getNickname().c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 1: " + std::string(sqlite3_errmsg(conn.get())));
        sqlite3_finalize(stmt);
        utils::g_db_pool.releaseConnection(conn);
        return false;
    }

    rc = sqlite3_bind_int(stmt, 2, user->getId());
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to bind parameter 2: " + std::string(sqlite3_errmsg(conn.get())));
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

bool UserRepository::deleteUser(int id) {
    auto conn = utils::g_db_pool.getConnection();
    if (!conn) {
        LOG_ERROR("Failed to get database connection");
        return false;
    }

    const char* sql = "DELETE FROM users WHERE id = ?";
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

} // namespace repository
