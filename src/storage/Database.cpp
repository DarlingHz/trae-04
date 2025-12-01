#include "storage/Database.hpp"
#include "utils/Logger.hpp"
#include <stdexcept>

Database::Database() : db_(nullptr), is_open_(false) {
}

Database::~Database() {
    close();
}

bool Database::open(const std::string& db_path) {
    if (is_open_) {
        LOG_WARNING("Database is already open");
        return true;
    }

    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to open database: " + std::string(sqlite3_errmsg(db_)));
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }

    is_open_ = true;
    LOG_INFO("Database opened successfully: " + db_path);
    return true;
}

void Database::close() {
    if (is_open_ && db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
        is_open_ = false;
        LOG_INFO("Database closed");
    }
}

bool Database::isOpen() const {
    return is_open_;
}

bool Database::execute(const std::string& sql) {
    if (!is_open_) {
        LOG_ERROR("Database is not open");
        return false;
    }

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to execute SQL: " + std::string(err_msg));
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

bool Database::query(const std::string& sql, ResultSet& result) {
    if (!is_open_) {
        LOG_ERROR("Database is not open");
        return false;
    }

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        LOG_ERROR("Failed to prepare SQL: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }

    result.clear();

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::map<std::string, std::string> row;
        int column_count = sqlite3_column_count(stmt);

        for (int i = 0; i < column_count; ++i) {
            const char* column_name = sqlite3_column_name(stmt, i);
            const char* column_value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));

            if (column_name != nullptr) {
                if (column_value != nullptr) {
                    row[column_name] = column_value;
                } else {
                    row[column_name] = "";
                }
            }
        }

        result.push_back(row);
    }

    if (rc != SQLITE_DONE) {
        LOG_ERROR("Failed to execute query: " + std::string(sqlite3_errmsg(db_)));
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

int64_t Database::getLastInsertId() const {
    if (!is_open_) {
        LOG_ERROR("Database is not open");
        return -1;
    }

    return sqlite3_last_insert_rowid(db_);
}

bool Database::beginTransaction() {
    return execute("BEGIN TRANSACTION;");
}

bool Database::commitTransaction() {
    return execute("COMMIT;");
}

bool Database::rollbackTransaction() {
    return execute("ROLLBACK;");
}

sqlite3* Database::getDb() const {
    return db_;
}
