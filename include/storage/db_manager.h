#pragma once

#include <sqlite3.h>
#include <string>
#include <mutex>

namespace storage {

class DBManager {
public:
    static DBManager& getInstance();
    
    bool init(const std::string& dbPath);
    
    sqlite3* getConnection();
    
    void close();
    
private:
    DBManager();
    ~DBManager();
    
    DBManager(const DBManager&) = delete;
    DBManager& operator=(const DBManager&) = delete;
    
    sqlite3* db_; // 数据库连接
    std::mutex mutex_; // 用于线程安全
};

} // namespace storage
