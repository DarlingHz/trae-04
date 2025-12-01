#include "tag_dao.h"
#include <iostream>

TagDao::TagDao(const std::string& db_path) : BaseDao(db_path) {
  CreateTagTable();
}

void TagDao::CreateTagTable() {
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return;
  }

  char* errmsg = nullptr;
  int rc = sqlite3_exec(db_, 
    "CREATE TABLE IF NOT EXISTS tags ("
    "tag_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "user_id INTEGER NOT NULL,"
    "name TEXT NOT NULL,"
    "created_at INTEGER NOT NULL,"
    "updated_at INTEGER NOT NULL,"
    "FOREIGN KEY(user_id) REFERENCES users(user_id) ON DELETE CASCADE,"
    "UNIQUE(user_id, name)"
    ");", 
    nullptr, nullptr, &errmsg);

  if (rc != SQLITE_OK) {
    std::cerr << "Failed to create tags table: " << errmsg << std::endl;
    sqlite3_free(errmsg);
  }
}

int TagDao::CreateTag(const Tag& tag) {
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return -1;
  }

  sqlite3_stmt* stmt = nullptr;
  int rc = sqlite3_prepare_v2(db_, "INSERT INTO tags (user_id, name, created_at, updated_at) VALUES (?, ?, ?, ?)", -1, &stmt, nullptr);

  if (rc != SQLITE_OK) {
    std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
    return -1;
  }

  // 绑定参数
  sqlite3_bind_int(stmt, 1, tag.user_id);
  sqlite3_bind_text(stmt, 2, tag.name.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_int64(stmt, 3, tag.created_at);
  sqlite3_bind_int64(stmt, 4, tag.updated_at);

  // 执行语句
  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db_) << std::endl;
    sqlite3_finalize(stmt);
    return -1;
  }

  // 获取最后插入的行ID
  int last_insert_id = sqlite3_last_insert_rowid(db_);

  // 清理语句
  sqlite3_finalize(stmt);
  return last_insert_id;
}

bool TagDao::UpdateTag(const Tag& tag) {
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return false;
  }

  sqlite3_stmt* stmt = nullptr;
  int rc = sqlite3_prepare_v2(db_, "UPDATE tags SET name = ?, updated_at = ? WHERE tag_id = ?", -1, &stmt, nullptr);

  if (rc != SQLITE_OK) {
    std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
    return false;
  }

  // 绑定参数
  sqlite3_bind_text(stmt, 1, tag.name.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_int64(stmt, 2, tag.updated_at);
  sqlite3_bind_int(stmt, 3, tag.tag_id);

  // 执行语句
  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db_) << std::endl;
    sqlite3_finalize(stmt);
    return false;
  }

  // 清理语句
  sqlite3_finalize(stmt);
  return true;
}

bool TagDao::DeleteTag(int tag_id) {
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return false;
  }

  sqlite3_stmt* stmt = nullptr;
  int rc = sqlite3_prepare_v2(db_, "DELETE FROM tags WHERE tag_id = ?", -1, &stmt, nullptr);

  if (rc != SQLITE_OK) {
    std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
    return false;
  }

  // 绑定参数
  sqlite3_bind_int(stmt, 1, tag_id);

  // 执行语句
  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db_) << std::endl;
    sqlite3_finalize(stmt);
    return false;
  }

  // 清理语句
  sqlite3_finalize(stmt);
  return true;
}

std::optional<Tag> TagDao::GetTagById(int tag_id) {
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return std::nullopt;
  }

  sqlite3_stmt* stmt = nullptr;
  int rc = sqlite3_prepare_v2(db_, "SELECT * FROM tags WHERE tag_id = ?", -1, &stmt, nullptr);

  if (rc != SQLITE_OK) {
    std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
    return std::nullopt;
  }

  // 绑定参数
  sqlite3_bind_int(stmt, 1, tag_id);

  // 执行语句
  rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW) {
    Tag tag;
    tag.tag_id = sqlite3_column_int(stmt, 0);
    tag.user_id = sqlite3_column_int(stmt, 1);
    tag.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    tag.created_at = sqlite3_column_int64(stmt, 3);
    tag.updated_at = sqlite3_column_int64(stmt, 4);

    // 清理语句
    sqlite3_finalize(stmt);
    return tag;
  } else if (rc == SQLITE_DONE) {
    // 没有找到标签
  } else {
    std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db_) << std::endl;
  }

  // 清理语句
  sqlite3_finalize(stmt);
  return std::nullopt;
}

std::optional<Tag> TagDao::GetTagByName(int user_id, const std::string& name) {
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return std::nullopt;
  }

  sqlite3_stmt* stmt = nullptr;
  int rc = sqlite3_prepare_v2(db_, "SELECT * FROM tags WHERE user_id = ? AND name = ?", -1, &stmt, nullptr);

  if (rc != SQLITE_OK) {
    std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
    return std::nullopt;
  }

  // 绑定参数
  sqlite3_bind_int(stmt, 1, user_id);
  sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT);

  // 执行语句
  rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW) {
    Tag tag;
    tag.tag_id = sqlite3_column_int(stmt, 0);
    tag.user_id = sqlite3_column_int(stmt, 1);
    tag.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    tag.created_at = sqlite3_column_int64(stmt, 3);
    tag.updated_at = sqlite3_column_int64(stmt, 4);

    // 清理语句
    sqlite3_finalize(stmt);
    return tag;
  } else if (rc == SQLITE_DONE) {
    // 没有找到标签
  } else {
    std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db_) << std::endl;
  }

  // 清理语句
  sqlite3_finalize(stmt);
  return std::nullopt;
}

std::vector<TagWithCount> TagDao::GetTagList(int user_id) {
  std::vector<TagWithCount> tags;
  
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return tags;
  }
  
  try {
    const char* sql = 
      "SELECT t.tag_id, t.name, COUNT(c.card_id) as card_count "
      "FROM tags t "
      "LEFT JOIN cards c ON c.tags LIKE '%' || t.tag_id || '%' AND c.is_deleted = 0 "
      "WHERE t.user_id = ? "
      "GROUP BY t.tag_id, t.name "
      "ORDER BY card_count DESC, t.name ASC";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
      std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
      return tags;
    }
    
    // 绑定参数
    sqlite3_bind_int(stmt, 1, user_id);
    
    // 执行查询
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
      TagWithCount tag;
      tag.tag_id = sqlite3_column_int(stmt, 0);
      tag.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
      tag.card_count = sqlite3_column_int(stmt, 2);
      
      tags.push_back(tag);
    }
    
    if (rc != SQLITE_DONE) {
      std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db_) << std::endl;
    }
    
    // 清理语句
    sqlite3_finalize(stmt);
  } catch (const std::exception& e) {
    std::cerr << "Failed to get tag list: " << e.what() << std::endl;
  }
  
  return tags;
}
