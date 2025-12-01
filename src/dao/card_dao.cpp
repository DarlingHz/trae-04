#include "card_dao.h"
#include <iostream>
#include <sstream>
#include "../util/time.h"

CardDao::CardDao(const std::string& db_path) : BaseDao(db_path) {
  CreateCardTable();
}

void CardDao::CreateCardTable() {
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return;
  }

  char* errmsg = nullptr;
  int rc = sqlite3_exec(db_, 
    "CREATE TABLE IF NOT EXISTS cards ("
    "card_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "user_id INTEGER NOT NULL,"
    "title TEXT NOT NULL,"
    "content TEXT,"
    "tags TEXT,"
    "is_pinned INTEGER NOT NULL DEFAULT 0,"
    "is_deleted INTEGER NOT NULL DEFAULT 0,"
    "created_at INTEGER NOT NULL,"
    "updated_at INTEGER NOT NULL,"
    "FOREIGN KEY(user_id) REFERENCES users(user_id) ON DELETE CASCADE"
    ");", 
    nullptr, nullptr, &errmsg);

  if (rc != SQLITE_OK) {
    std::cerr << "Failed to create cards table: " << errmsg << std::endl;
    sqlite3_free(errmsg);
  }
}

int CardDao::CreateCard(const Card& card) {
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return -1;
  }

  sqlite3_stmt* stmt = nullptr;
  int rc = sqlite3_prepare_v2(db_, "INSERT INTO cards (user_id, title, content, tags, is_pinned, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?)", -1, &stmt, nullptr);

  if (rc != SQLITE_OK) {
    std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
    return -1;
  }

  // 将tags向量转换为逗号分隔的字符串
  std::string tags_string;
  for (size_t i = 0; i < card.tags.size(); ++i) {
    if (i > 0) tags_string += ",";
    tags_string += card.tags[i];
  }

  // 绑定参数
  sqlite3_bind_int(stmt, 1, card.user_id);
  sqlite3_bind_text(stmt, 2, card.title.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 3, card.content.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 4, tags_string.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_int(stmt, 5, card.is_pinned);
  sqlite3_bind_int64(stmt, 6, card.created_at);
  sqlite3_bind_int64(stmt, 7, card.updated_at);

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

bool CardDao::UpdateCard(const Card& card) {
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return false;
  }

  sqlite3_stmt* stmt = nullptr;
  int rc = sqlite3_prepare_v2(db_, "UPDATE cards SET title = ?, content = ?, tags = ?, is_pinned = ?, updated_at = ? WHERE card_id = ?", -1, &stmt, nullptr);

  if (rc != SQLITE_OK) {
    std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
    return false;
  }

  // 将tags向量转换为逗号分隔的字符串
  std::string tags_string;
  for (size_t i = 0; i < card.tags.size(); ++i) {
    if (i > 0) tags_string += ",";
    tags_string += card.tags[i];
  }

  // 绑定参数
  sqlite3_bind_text(stmt, 1, card.title.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, card.content.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 3, tags_string.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_int(stmt, 4, card.is_pinned);
  sqlite3_bind_int64(stmt, 5, card.updated_at);
  sqlite3_bind_int(stmt, 6, card.card_id);

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

bool CardDao::DeleteCard(int card_id) {
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return false;
  }

  sqlite3_stmt* stmt = nullptr;
  int rc = sqlite3_prepare_v2(db_, "UPDATE cards SET is_deleted = 1, updated_at = ? WHERE card_id = ?", -1, &stmt, nullptr);

  if (rc != SQLITE_OK) {
    std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
    return false;
  }

  // 绑定参数
  sqlite3_bind_int64(stmt, 1, TimeUtil::GetCurrentTimestamp());
  sqlite3_bind_int(stmt, 2, card_id);

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

std::optional<Card> CardDao::GetCardById(int card_id) {
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return std::nullopt;
  }

  sqlite3_stmt* stmt = nullptr;
  int rc = sqlite3_prepare_v2(db_, "SELECT * FROM cards WHERE card_id = ? AND is_deleted = 0", -1, &stmt, nullptr);

  if (rc != SQLITE_OK) {
    std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
    return std::nullopt;
  }

  // 绑定参数
  sqlite3_bind_int(stmt, 1, card_id);

  // 执行语句
  rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW) {
      Card card;
      card.card_id = sqlite3_column_int(stmt, 0);
      card.user_id = sqlite3_column_int(stmt, 1);
      card.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
      card.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
      
      // 解析tags字符串为vector<string>
      const char* tags_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
      if (tags_str != nullptr) {
        std::string tags_string(tags_str);
        std::vector<std::string> tags_vector;
        size_t pos = 0;
        std::string token;
        while ((pos = tags_string.find(',')) != std::string::npos) {
          token = tags_string.substr(0, pos);
          tags_vector.push_back(token);
          tags_string.erase(0, pos + 1);
        }
        // 添加最后一个标签
        if (!tags_string.empty()) {
          tags_vector.push_back(tags_string);
        }
        card.tags = tags_vector;
      } else {
        card.tags = std::vector<std::string>();
      }
      
      card.is_pinned = sqlite3_column_int(stmt, 5);
      card.is_deleted = sqlite3_column_int(stmt, 6);
      card.created_at = sqlite3_column_int64(stmt, 7);
      card.updated_at = sqlite3_column_int64(stmt, 8);

    // 清理语句
    sqlite3_finalize(stmt);
    return card;
  } else if (rc == SQLITE_DONE) {
    // 没有找到卡片
  } else {
    std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db_) << std::endl;
  }

  // 清理语句
  sqlite3_finalize(stmt);
  return std::nullopt;
}

std::vector<Card> CardDao::GetCardList(int user_id, int offset, int limit, const std::string& sort, 
                                            const std::vector<int>& tag_ids, const std::string& search) {
  std::vector<Card> cards;
  
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return cards;
  }
  
  try {
    std::stringstream ss;
    ss << "SELECT * FROM cards WHERE user_id = ? AND is_deleted = 0";
    
    // 添加标签过滤
    if (!tag_ids.empty()) {
      ss << " AND tags LIKE '%";
      for (size_t i = 0; i < tag_ids.size(); ++i) {
        if (i > 0) ss << ",%";
        ss << tag_ids[i];
      }
      ss << "%'";
    }
    
    // 添加搜索过滤
    if (!search.empty()) {
      ss << " AND (title LIKE '%" << search << "%' OR content LIKE '%" << search << "%')";
    }
    
    // 添加排序
    if (sort == "created_at") {
      ss << " ORDER BY is_pinned DESC, created_at DESC";
    } else if (sort == "updated_at") {
      ss << " ORDER BY is_pinned DESC, updated_at DESC";
    } else {
      ss << " ORDER BY is_pinned DESC, updated_at DESC";
    }
    
    // 添加分页
    ss << " LIMIT ? OFFSET ?";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, ss.str().c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
      std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
      return cards;
    }
    
    // 绑定参数
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, limit);
    sqlite3_bind_int(stmt, 3, offset);
    
    // 执行查询
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
      Card card;
      card.card_id = sqlite3_column_int(stmt, 0);
      card.user_id = sqlite3_column_int(stmt, 1);
      card.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
      card.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
      // 解析tags字符串为vector<string>
      const char* tags_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
      if (tags_str != nullptr) {
        std::string tags_string(tags_str);
        std::vector<std::string> tags_vector;
        size_t pos = 0;
        std::string token;
        while ((pos = tags_string.find(',')) != std::string::npos) {
          token = tags_string.substr(0, pos);
          tags_vector.push_back(token);
          tags_string.erase(0, pos + 1);
        }
        // 添加最后一个标签
        if (!tags_string.empty()) {
          tags_vector.push_back(tags_string);
        }
        card.tags = tags_vector;
      } else {
        card.tags = std::vector<std::string>();
      }
      card.is_pinned = sqlite3_column_int(stmt, 5);
      card.is_deleted = sqlite3_column_int(stmt, 6);
      card.created_at = sqlite3_column_int64(stmt, 7);
      card.updated_at = sqlite3_column_int64(stmt, 8);
      
      cards.push_back(card);
    }
    
    if (rc != SQLITE_DONE) {
      std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db_) << std::endl;
    }
    
    // 清理语句
    sqlite3_finalize(stmt);
  } catch (const std::exception& e) {
    std::cerr << "Failed to get card list: " << e.what() << std::endl;
  }
  
  return cards;
}

int CardDao::GetCardCount(int user_id, const std::vector<int>& tag_ids, const std::string& search) {
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return 0;
  }
  
  try {
    std::stringstream ss;
    ss << "SELECT COUNT(*) FROM cards WHERE user_id = ? AND is_deleted = 0";
    
    // 添加标签过滤
    if (!tag_ids.empty()) {
      ss << " AND tags LIKE '%";
      for (size_t i = 0; i < tag_ids.size(); ++i) {
        if (i > 0) ss << ",%";
        ss << tag_ids[i];
      }
      ss << "%'";
    }
    
    // 添加搜索过滤
    if (!search.empty()) {
      ss << " AND (title LIKE '%" << search << "%' OR content LIKE '%" << search << "%')";
    }
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, ss.str().c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
      std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
      return 0;
    }
    
    // 绑定参数
    sqlite3_bind_int(stmt, 1, user_id);
    
    // 执行查询
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      int count = sqlite3_column_int(stmt, 0);
      sqlite3_finalize(stmt);
      return count;
    } else if (rc == SQLITE_DONE) {
      // 没有找到记录
    } else {
      std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db_) << std::endl;
    }
    
    // 清理语句
    sqlite3_finalize(stmt);
  } catch (const std::exception& e) {
    std::cerr << "Failed to get card count: " << e.what() << std::endl;
  }
  
  return 0;
}
