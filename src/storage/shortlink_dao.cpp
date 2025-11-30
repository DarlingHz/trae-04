#include "storage/shortlink_dao.h"
#include "storage/db_manager.h"
#include "utils/logger.h"
#include <sqlite3.h>

namespace storage {

bool ShortLinkDAO::createShortLink(const model::ShortLink& link) {
    sqlite3* db = DBManager::getInstance().getConnection();
    if (db == nullptr) {
        LOG_ERROR("Database connection is null");
        return false;
    }
    
    const char* sql = R"(
        INSERT INTO short_links (
            long_url, short_code, custom_alias, create_time, expire_time, is_enabled, visit_count
        ) VALUES (?, ?, ?, ?, ?, ?, ?)
    )";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: ", sqlite3_errmsg(db));
        return false;
    }
    
    // 绑定参数
    sqlite3_bind_text(stmt, 1, link.long_url.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, link.short_code.c_str(), -1, SQLITE_TRANSIENT);
    
    if (link.custom_alias.empty()) {
        sqlite3_bind_null(stmt, 3);
    } else {
        sqlite3_bind_text(stmt, 3, link.custom_alias.c_str(), -1, SQLITE_TRANSIENT);
    }
    
    sqlite3_bind_int64(stmt, 4, link.create_time);
    sqlite3_bind_int64(stmt, 5, link.expire_time);
    sqlite3_bind_int(stmt, 6, link.is_enabled ? 1 : 0);
    sqlite3_bind_int64(stmt, 7, link.visit_count);
    
    // 执行语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute statement: ", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }
    
    // 清理
    sqlite3_finalize(stmt);
    
    LOG_INFO("Short link created successfully: ", link.short_code);
    
    return true;
}

std::optional<model::ShortLink> ShortLinkDAO::findShortLinkById(uint64_t id) {
    sqlite3* db = DBManager::getInstance().getConnection();
    if (db == nullptr) {
        LOG_ERROR("Database connection is null");
        return std::nullopt;
    }
    
    const char* sql = R"(
        SELECT id, long_url, short_code, custom_alias, create_time, expire_time, is_enabled, visit_count
        FROM short_links WHERE id = ?
    )";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: ", sqlite3_errmsg(db));
        return std::nullopt;
    }
    
    // 绑定参数
    sqlite3_bind_int64(stmt, 1, id);
    
    // 执行查询
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }
    
    // 解析结果
    model::ShortLink link;
    link.id = sqlite3_column_int64(stmt, 0);
    link.long_url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    link.short_code = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    
    if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) {
        link.custom_alias = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    } else {
        link.custom_alias = "";
    }
    
    link.create_time = sqlite3_column_int64(stmt, 4);
    link.expire_time = sqlite3_column_int64(stmt, 5);
    link.is_enabled = sqlite3_column_int(stmt, 6) == 1;
    link.visit_count = sqlite3_column_int64(stmt, 7);
    
    // 清理
    sqlite3_finalize(stmt);
    
    return link;
}

std::optional<model::ShortLink> ShortLinkDAO::findShortLinkByCode(const std::string& shortCode) {
    sqlite3* db = DBManager::getInstance().getConnection();
    if (db == nullptr) {
        LOG_ERROR("Database connection is null");
        return std::nullopt;
    }
    
    const char* sql = R"(
        SELECT id, long_url, short_code, custom_alias, create_time, expire_time, is_enabled, visit_count
        FROM short_links WHERE short_code = ?
    )";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: ", sqlite3_errmsg(db));
        return std::nullopt;
    }
    
    // 绑定参数
    sqlite3_bind_text(stmt, 1, shortCode.c_str(), -1, SQLITE_TRANSIENT);
    
    // 执行查询
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }
    
    // 解析结果
    model::ShortLink link;
    link.id = sqlite3_column_int64(stmt, 0);
    link.long_url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    link.short_code = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    
    if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) {
        link.custom_alias = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    } else {
        link.custom_alias = "";
    }
    
    link.create_time = sqlite3_column_int64(stmt, 4);
    link.expire_time = sqlite3_column_int64(stmt, 5);
    link.is_enabled = sqlite3_column_int(stmt, 6) == 1;
    link.visit_count = sqlite3_column_int64(stmt, 7);
    
    // 清理
    sqlite3_finalize(stmt);
    
    return link;
}

std::optional<model::ShortLink> ShortLinkDAO::findShortLinkByAlias(const std::string& alias) {
    sqlite3* db = DBManager::getInstance().getConnection();
    if (db == nullptr) {
        LOG_ERROR("Database connection is null");
        return std::nullopt;
    }
    
    const char* sql = R"(
        SELECT id, long_url, short_code, custom_alias, create_time, expire_time, is_enabled, visit_count
        FROM short_links WHERE custom_alias = ?
    )";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: ", sqlite3_errmsg(db));
        return std::nullopt;
    }
    
    // 绑定参数
    sqlite3_bind_text(stmt, 1, alias.c_str(), -1, SQLITE_TRANSIENT);
    
    // 执行查询
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }
    
    // 解析结果
    model::ShortLink link;
    link.id = sqlite3_column_int64(stmt, 0);
    link.long_url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    link.short_code = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    
    if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) {
        link.custom_alias = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    } else {
        link.custom_alias = "";
    }
    
    link.create_time = sqlite3_column_int64(stmt, 4);
    link.expire_time = sqlite3_column_int64(stmt, 5);
    link.is_enabled = sqlite3_column_int(stmt, 6) == 1;
    link.visit_count = sqlite3_column_int64(stmt, 7);
    
    // 清理
    sqlite3_finalize(stmt);
    
    return link;
}

bool ShortLinkDAO::updateShortLink(const model::ShortLink& link) {
    sqlite3* db = DBManager::getInstance().getConnection();
    if (db == nullptr) {
        LOG_ERROR("Database connection is null");
        return false;
    }
    
    const char* sql = R"(
        UPDATE short_links SET 
            long_url = ?, short_code = ?, custom_alias = ?, create_time = ?, expire_time = ?, 
            is_enabled = ?, visit_count = ? 
        WHERE id = ?
    )";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: ", sqlite3_errmsg(db));
        return false;
    }
    
    // 绑定参数
    sqlite3_bind_text(stmt, 1, link.long_url.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, link.short_code.c_str(), -1, SQLITE_TRANSIENT);
    
    if (link.custom_alias.empty()) {
        sqlite3_bind_null(stmt, 3);
    } else {
        sqlite3_bind_text(stmt, 3, link.custom_alias.c_str(), -1, SQLITE_TRANSIENT);
    }
    
    sqlite3_bind_int64(stmt, 4, link.create_time);
    sqlite3_bind_int64(stmt, 5, link.expire_time);
    sqlite3_bind_int(stmt, 6, link.is_enabled ? 1 : 0);
    sqlite3_bind_int64(stmt, 7, link.visit_count);
    sqlite3_bind_int64(stmt, 8, link.id);
    
    // 执行语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute statement: ", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }
    
    // 清理
    sqlite3_finalize(stmt);
    
    LOG_INFO("Short link updated successfully: ", link.id);
    
    return true;
}

bool ShortLinkDAO::incrementVisitCount(uint64_t id) {
    sqlite3* db = DBManager::getInstance().getConnection();
    if (db == nullptr) {
        LOG_ERROR("Database connection is null");
        return false;
    }
    
    const char* sql = R"(
        UPDATE short_links SET visit_count = visit_count + 1 WHERE id = ?
    )";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: ", sqlite3_errmsg(db));
        return false;
    }
    
    // 绑定参数
    sqlite3_bind_int64(stmt, 1, id);
    
    // 执行语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute statement: ", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }
    
    // 清理
    sqlite3_finalize(stmt);
    
    return true;
}

bool ShortLinkDAO::addVisitLog(const model::VisitLog& log) {
    sqlite3* db = DBManager::getInstance().getConnection();
    if (db == nullptr) {
        LOG_ERROR("Database connection is null");
        return false;
    }
    
    const char* sql = R"(
        INSERT INTO visit_logs (link_id, ip, user_agent, visit_time) VALUES (?, ?, ?, ?)
    )";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: ", sqlite3_errmsg(db));
        return false;
    }
    
    // 绑定参数
    sqlite3_bind_int64(stmt, 1, log.link_id);
    sqlite3_bind_text(stmt, 2, log.ip.c_str(), -1, SQLITE_TRANSIENT);
    
    if (log.user_agent.empty()) {
        sqlite3_bind_null(stmt, 3);
    } else {
        sqlite3_bind_text(stmt, 3, log.user_agent.c_str(), -1, SQLITE_TRANSIENT);
    }
    
    sqlite3_bind_int64(stmt, 4, log.visit_time);
    
    // 执行语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute statement: ", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }
    
    // 清理
    sqlite3_finalize(stmt);
    
    return true;
}

std::vector<model::VisitLog> ShortLinkDAO::getVisitLogs(uint64_t linkId, size_t limit) {
    std::vector<model::VisitLog> logs;
    
    sqlite3* db = DBManager::getInstance().getConnection();
    if (db == nullptr) {
        LOG_ERROR("Database connection is null");
        return logs;
    }
    
    const char* sql = R"(
        SELECT id, link_id, ip, user_agent, visit_time 
        FROM visit_logs WHERE link_id = ? ORDER BY visit_time DESC LIMIT ?
    )";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: ", sqlite3_errmsg(db));
        return logs;
    }
    
    // 绑定参数
    sqlite3_bind_int64(stmt, 1, linkId);
    sqlite3_bind_int(stmt, 2, static_cast<int>(limit));
    
    // 执行查询
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        model::VisitLog log;
        log.id = sqlite3_column_int64(stmt, 0);
        log.link_id = sqlite3_column_int64(stmt, 1);
        log.ip = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        
        if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) {
            log.user_agent = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        } else {
            log.user_agent = "";
        }
        
        log.visit_time = sqlite3_column_int64(stmt, 4);
        
        logs.push_back(log);
    }
    
    // 清理
    sqlite3_finalize(stmt);
    
    return logs;
}

bool ShortLinkDAO::disableShortLink(uint64_t id) {
    sqlite3* db = DBManager::getInstance().getConnection();
    if (db == nullptr) {
        LOG_ERROR("Database connection is null");
        return false;
    }
    
    const char* sql = R"(
        UPDATE short_links SET is_enabled = 0 WHERE id = ?
    )";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: ", sqlite3_errmsg(db));
        return false;
    }
    
    // 绑定参数
    sqlite3_bind_int64(stmt, 1, id);
    
    // 执行语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute statement: ", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }
    
    // 清理
    sqlite3_finalize(stmt);
    
    LOG_INFO("Short link disabled successfully: ", id);
    
    return true;
}

} // namespace storage
