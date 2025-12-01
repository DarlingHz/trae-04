#include "storage/ApiKeyRepository.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"

using namespace Utils;

ApiKeyRepository::ApiKeyRepository(Database& db) : db_(db) {
}

ApiKeyRepository::~ApiKeyRepository() {
}

bool ApiKeyRepository::createTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS api_keys (
            key_id INTEGER PRIMARY KEY AUTOINCREMENT,
            client_id INTEGER NOT NULL,
            api_key TEXT NOT NULL UNIQUE,
            expired_at TEXT,
            is_revoked INTEGER NOT NULL DEFAULT 0,
            created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
            updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (client_id) REFERENCES clients (client_id)
        );
    )";

    return db_.execute(sql);
}

bool ApiKeyRepository::insert(ApiKey& api_key) {
    std::string sql = R"(
        INSERT INTO api_keys (client_id, api_key, expired_at, is_revoked, created_at, updated_at) 
        VALUES (?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_.getDb(), sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare insert api key statement: " + std::string(sqlite3_errmsg(db_.getDb())));
        return false;
    }

    sqlite3_bind_int64(stmt, 1, api_key.client_id);
    sqlite3_bind_text(stmt, 2, api_key.api_key.c_str(), -1, SQLITE_TRANSIENT);

    if (api_key.expired_at.empty()) {
        sqlite3_bind_null(stmt, 3);
    } else {
        sqlite3_bind_text(stmt, 3, api_key.expired_at.c_str(), -1, SQLITE_TRANSIENT);
    }

    sqlite3_bind_int(stmt, 4, api_key.is_revoked ? 1 : 0);
    sqlite3_bind_text(stmt, 5, api_key.created_at.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, api_key.updated_at.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to insert api key: " + std::string(sqlite3_errmsg(db_.getDb())));
        sqlite3_finalize(stmt);
        return false;
    }

    api_key.key_id = db_.getLastInsertId();

    sqlite3_finalize(stmt);
    return true;
}

bool ApiKeyRepository::update(const ApiKey& api_key) {
    std::string sql = R"(
        UPDATE api_keys SET client_id = ?, api_key = ?, expired_at = ?, is_revoked = ?, updated_at = ? 
        WHERE key_id = ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_.getDb(), sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare update api key statement: " + std::string(sqlite3_errmsg(db_.getDb())));
        return false;
    }

    sqlite3_bind_int64(stmt, 1, api_key.client_id);
    sqlite3_bind_text(stmt, 2, api_key.api_key.c_str(), -1, SQLITE_TRANSIENT);

    if (api_key.expired_at.empty()) {
        sqlite3_bind_null(stmt, 3);
    } else {
        sqlite3_bind_text(stmt, 3, api_key.expired_at.c_str(), -1, SQLITE_TRANSIENT);
    }

    sqlite3_bind_int(stmt, 4, api_key.is_revoked ? 1 : 0);
    sqlite3_bind_text(stmt, 5, api_key.updated_at.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 6, api_key.key_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to update api key: " + std::string(sqlite3_errmsg(db_.getDb())));
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool ApiKeyRepository::revoke(int64_t key_id) {
    std::string sql = "UPDATE api_keys SET is_revoked = 1, updated_at = ? WHERE key_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_.getDb(), sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare revoke api key statement: " + std::string(sqlite3_errmsg(db_.getDb())));
        return false;
    }

    std::string current_time = getCurrentTimeStr();
    sqlite3_bind_text(stmt, 1, current_time.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 2, key_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to revoke api key: " + std::string(sqlite3_errmsg(db_.getDb())));
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool ApiKeyRepository::getById(int64_t key_id, ApiKey& api_key) {
    std::string sql = "SELECT * FROM api_keys WHERE key_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_.getDb(), sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare get api key by id statement: " + std::string(sqlite3_errmsg(db_.getDb())));
        return false;
    }

    sqlite3_bind_int64(stmt, 1, key_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        if (rc != SQLITE_DONE) {
            LOG_ERROR("Failed to get api key by id: " + std::string(sqlite3_errmsg(db_.getDb())));
        }
        sqlite3_finalize(stmt);
        return false;
    }

    api_key.key_id = sqlite3_column_int64(stmt, 0);
    api_key.client_id = sqlite3_column_int64(stmt, 1);
    api_key.api_key = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

    if (sqlite3_column_type(stmt, 3) == SQLITE_NULL) {
        api_key.expired_at = "";
    } else {
        api_key.expired_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    }

    api_key.is_revoked = (sqlite3_column_int(stmt, 4) == 1);
    api_key.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    api_key.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

    sqlite3_finalize(stmt);
    return true;
}

bool ApiKeyRepository::getByKey(const std::string& api_key_str, ApiKey& api_key) {
    std::string sql = "SELECT * FROM api_keys WHERE api_key = ?;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_.getDb(), sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare get api key by key statement: " + std::string(sqlite3_errmsg(db_.getDb())));
        return false;
    }

    sqlite3_bind_text(stmt, 1, api_key_str.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        if (rc != SQLITE_DONE) {
            LOG_ERROR("Failed to get api key by key: " + std::string(sqlite3_errmsg(db_.getDb())));
        }
        sqlite3_finalize(stmt);
        return false;
    }

    api_key.key_id = sqlite3_column_int64(stmt, 0);
    api_key.client_id = sqlite3_column_int64(stmt, 1);
    api_key.api_key = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

    if (sqlite3_column_type(stmt, 3) == SQLITE_NULL) {
        api_key.expired_at = "";
    } else {
        api_key.expired_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    }

    api_key.is_revoked = (sqlite3_column_int(stmt, 4) == 1);
    api_key.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    api_key.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

    sqlite3_finalize(stmt);
    return true;
}

bool ApiKeyRepository::getByClientId(int64_t client_id, std::vector<ApiKey>& api_keys) {
    std::string sql = "SELECT * FROM api_keys WHERE client_id = ? ORDER BY key_id;";

    Database::ResultSet result;
    if (!db_.query(sql, result)) {
        LOG_ERROR("Failed to get api keys by client id");
        return false;
    }

    api_keys.clear();
    for (const auto& row : result) {
        ApiKey api_key;
        api_key.key_id = std::stoll(row.at("key_id"));
        api_key.client_id = std::stoll(row.at("client_id"));
        api_key.api_key = row.at("api_key");
        api_key.expired_at = row.count("expired_at") ? row.at("expired_at") : "";
        api_key.is_revoked = (row.at("is_revoked") == "1");
        api_key.created_at = row.at("created_at");
        api_key.updated_at = row.at("updated_at");
        api_keys.push_back(api_key);
    }

    return true;
}
