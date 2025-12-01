#include "storage/ClientRepository.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"

using namespace Utils;

ClientRepository::ClientRepository(Database& db) : db_(db) {
}

ClientRepository::~ClientRepository() {
}

bool ClientRepository::createTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS clients (
            client_id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            contact_email TEXT NOT NULL,
            daily_quota INTEGER NOT NULL DEFAULT 10000,
            per_minute_quota INTEGER NOT NULL DEFAULT 200,
            is_enabled INTEGER NOT NULL DEFAULT 1,
            created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
            updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
        );
    )";

    return db_.execute(sql);
}

bool ClientRepository::insert(Client& client) {
    std::string sql = R"(
        INSERT INTO clients (name, contact_email, daily_quota, per_minute_quota, is_enabled, created_at, updated_at) 
        VALUES (?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_.getDb(), sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare insert client statement: " + std::string(sqlite3_errmsg(db_.getDb())));
        return false;
    }

    sqlite3_bind_text(stmt, 1, client.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, client.contact_email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, client.daily_quota);
    sqlite3_bind_int(stmt, 4, client.per_minute_quota);
    sqlite3_bind_int(stmt, 5, client.is_enabled ? 1 : 0);
    sqlite3_bind_text(stmt, 6, client.created_at.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, client.updated_at.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to insert client: " + std::string(sqlite3_errmsg(db_.getDb())));
        sqlite3_finalize(stmt);
        return false;
    }

    client.client_id = db_.getLastInsertId();

    sqlite3_finalize(stmt);
    return true;
}

bool ClientRepository::update(const Client& client) {
    std::string sql = R"(
        UPDATE clients SET name = ?, contact_email = ?, daily_quota = ?, per_minute_quota = ?, is_enabled = ?, updated_at = ? 
        WHERE client_id = ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_.getDb(), sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare update client statement: " + std::string(sqlite3_errmsg(db_.getDb())));
        return false;
    }

    sqlite3_bind_text(stmt, 1, client.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, client.contact_email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, client.daily_quota);
    sqlite3_bind_int(stmt, 4, client.per_minute_quota);
    sqlite3_bind_int(stmt, 5, client.is_enabled ? 1 : 0);
    sqlite3_bind_text(stmt, 6, client.updated_at.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 7, client.client_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to update client: " + std::string(sqlite3_errmsg(db_.getDb())));
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool ClientRepository::remove(int64_t client_id) {
    std::string sql = "UPDATE clients SET is_enabled = 0, updated_at = ? WHERE client_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_.getDb(), sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare delete client statement: " + std::string(sqlite3_errmsg(db_.getDb())));
        return false;
    }

    std::string current_time = getCurrentTimeStr();
    sqlite3_bind_text(stmt, 1, current_time.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 2, client_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to delete client: " + std::string(sqlite3_errmsg(db_.getDb())));
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool ClientRepository::getById(int64_t client_id, Client& client) {
    std::string sql = "SELECT * FROM clients WHERE client_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_.getDb(), sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare get client by id statement: " + std::string(sqlite3_errmsg(db_.getDb())));
        return false;
    }

    sqlite3_bind_int64(stmt, 1, client_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        if (rc != SQLITE_DONE) {
            LOG_ERROR("Failed to get client by id: " + std::string(sqlite3_errmsg(db_.getDb())));
        }
        sqlite3_finalize(stmt);
        return false;
    }

    client.client_id = sqlite3_column_int64(stmt, 0);
    client.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    client.contact_email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    client.daily_quota = sqlite3_column_int(stmt, 3);
    client.per_minute_quota = sqlite3_column_int(stmt, 4);
    client.is_enabled = (sqlite3_column_int(stmt, 5) == 1);
    client.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    client.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

    sqlite3_finalize(stmt);
    return true;
}

bool ClientRepository::getAll(std::vector<Client>& clients) {
    std::string sql = "SELECT * FROM clients ORDER BY client_id;";

    Database::ResultSet result;
    if (!db_.query(sql, result)) {
        LOG_ERROR("Failed to get all clients");
        return false;
    }

    clients.clear();
    for (const auto& row : result) {
        Client client;
        client.client_id = std::stoll(row.at("client_id"));
        client.name = row.at("name");
        client.contact_email = row.at("contact_email");
        client.daily_quota = std::stoul(row.at("daily_quota"));
        client.per_minute_quota = std::stoul(row.at("per_minute_quota"));
        client.is_enabled = (row.at("is_enabled") == "1");
        client.created_at = row.at("created_at");
        client.updated_at = row.at("updated_at");
        clients.push_back(client);
    }

    return true;
}
