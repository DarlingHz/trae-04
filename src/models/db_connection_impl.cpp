#include "db_connection.h"
#include "../common/logger.h"
#include <unordered_map>
#include <vector>
#include <sstream>

namespace models {

// 内存数据库记录类型
typedef std::unordered_map<std::string, std::string> Record;
// 内存数据库表类型
typedef std::vector<Record> Table;
// 内存数据库类型
typedef std::unordered_map<std::string, Table> Database;

// 内存数据库单例
class MemoryDatabase {
public:
    static MemoryDatabase& getInstance() {
        static MemoryDatabase instance;
        return instance;
    }

    // 获取表
    Table& getTable(const std::string& tableName) {
        std::lock_guard<std::mutex> lock(mutex_);
        return tables_[tableName];
    }

    // 添加记录
    void addRecord(const std::string& tableName, const Record& record) {
        std::lock_guard<std::mutex> lock(mutex_);
        tables_[tableName].push_back(record);
    }

    // 获取记录数
    size_t getRecordCount(const std::string& tableName) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = tables_.find(tableName);
        if (it != tables_.end()) {
            return it->second.size();
        }
        return 0;
    }

private:
    MemoryDatabase() {
        // 初始化一些默认表
        createDefaultTables();
    }

    void createDefaultTables() {
        // 用户表
        tables_["users"];
        // 题目表
        tables_["problems"];
        // 刷题记录表
        tables_["records"];
    }

    std::unordered_map<std::string, Table> tables_;
    std::mutex mutex_;
};

// 内存数据库连接实现
class MemoryDBConnection : public DBConnection {
public:
    MemoryDBConnection() : currentRow_(0), isValid_(true) {
    }

    ~MemoryDBConnection() override {
        close();
    }

    bool executeQuery(const std::string& query) override {
        try {
            // 简单的SQL解析，仅支持SELECT语句的基本形式
            currentTable_.clear();
            currentRow_ = 0;
            
            // 解析表名
            std::string tableName = parseTableName(query);
            if (tableName.empty()) {
                common::g_logger.error("Failed to parse table name from query: %s", query.c_str());
                return false;
            }

            // 获取表数据
            MemoryDatabase& db = MemoryDatabase::getInstance();
            Table& table = db.getTable(tableName);
            
            // 简单的WHERE条件过滤（仅支持简单的等值条件）
            size_t wherePos = query.find("WHERE");
            if (wherePos != std::string::npos) {
                std::string condition = query.substr(wherePos + 5);
                filterByCondition(table, condition);
            } else {
                currentTable_ = table;
            }

            common::g_logger.debug("Query executed: %s, returned %zu rows", query.c_str(), currentTable_.size());
            return true;
        } catch (const std::exception& e) {
            common::g_logger.error("Error executing query: %s, exception: %s", query.c_str(), e.what());
            return false;
        }
    }

    int executeUpdate(const std::string& query) override {
        try {
            MemoryDatabase& db = MemoryDatabase::getInstance();
            
            if (query.find("INSERT") != std::string::npos) {
                // 处理插入操作
                std::string tableName = parseTableName(query);
                Record record = parseInsertValues(query);
                
                // 为记录生成ID
                long long id = db.getRecordCount(tableName) + 1;
                record["id"] = std::to_string(id);
                lastInsertId_ = id;
                
                db.addRecord(tableName, record);
                common::g_logger.debug("Insert executed: %s, id: %lld", query.c_str(), id);
                return 1;
            } else if (query.find("UPDATE") != std::string::npos) {
                // 简单的更新操作模拟
                common::g_logger.debug("Update executed: %s", query.c_str());
                return 0;
            } else if (query.find("DELETE") != std::string::npos) {
                // 简单的删除操作模拟
                common::g_logger.debug("Delete executed: %s", query.c_str());
                return 0;
            }
            
            return 0;
        } catch (const std::exception& e) {
            common::g_logger.error("Error executing update: %s, exception: %s", query.c_str(), e.what());
            return -1;
        }
    }

    long long getLastInsertId() override {
        return lastInsertId_;
    }

    bool hasNext() override {
        return currentRow_ < currentTable_.size();
    }

    bool next() override {
        if (currentRow_ < currentTable_.size()) {
            currentRow_++;
            return true;
        }
        return false;
    }

    std::string getString(const std::string& field) override {
        if (currentRow_ > 0 && currentRow_ <= currentTable_.size()) {
            auto& record = currentTable_[currentRow_ - 1];
            auto it = record.find(field);
            if (it != record.end()) {
                return it->second;
            }
        }
        return "";
    }

    int getInt(const std::string& field) override {
        try {
            return std::stoi(getString(field));
        } catch (...) {
            return 0;
        }
    }

    long long getLongLong(const std::string& field) override {
        try {
            return std::stoll(getString(field));
        } catch (...) {
            return 0;
        }
    }

    double getDouble(const std::string& field) override {
        try {
            return std::stod(getString(field));
        } catch (...) {
            return 0.0;
        }
    }

    bool getBool(const std::string& field) override {
        std::string value = getString(field);
        return value == "true" || value == "1" || value == "yes";
    }

    void close() override {
        isValid_ = false;
        currentTable_.clear();
    }

    bool isValid() override {
        return isValid_;
    }

private:
    // 解析表名
    std::string parseTableName(const std::string& query) {
        std::stringstream ss(query);
        std::string token;
        std::vector<std::string> tokens;
        
        while (ss >> token) {
            tokens.push_back(token);
        }
        
        for (size_t i = 0; i < tokens.size(); i++) {
            if (tokens[i] == "FROM" || tokens[i] == "from" || 
                tokens[i] == "INSERT" || tokens[i] == "insert" ||
                tokens[i] == "UPDATE" || tokens[i] == "update" ||
                tokens[i] == "DELETE" || tokens[i] == "delete") {
                if (i + 1 < tokens.size()) {
                    // 移除可能的表名引号
                    std::string tableName = tokens[i + 1];
                    if (!tableName.empty() && (tableName[0] == '`' || tableName[0] == '"' || tableName[0] == '\'')) {
                        tableName = tableName.substr(1, tableName.size() - 2);
                    }
                    return tableName;
                }
            }
        }
        
        return "";
    }

    // 解析插入值
    Record parseInsertValues(const std::string& query) {
        Record record;
        
        // 简单解析，仅支持INSERT INTO table (col1, col2) VALUES (val1, val2) 格式
        size_t valuesPos = query.find("VALUES");
        if (valuesPos != std::string::npos) {
            std::string valuesPart = query.substr(valuesPos + 6);
            
            // 提取字段名
            size_t openParenPos = query.find("(");
            size_t closeParenPos = query.find(")");
            if (openParenPos != std::string::npos && closeParenPos != std::string::npos && openParenPos < closeParenPos) {
                std::string fieldsPart = query.substr(openParenPos + 1, closeParenPos - openParenPos - 1);
                std::vector<std::string> fields = splitString(fieldsPart, ',');
                
                // 提取值
                openParenPos = valuesPart.find("(");
                closeParenPos = valuesPart.find(")");
                if (openParenPos != std::string::npos && closeParenPos != std::string::npos) {
                    std::string valuesStr = valuesPart.substr(openParenPos + 1, closeParenPos - openParenPos - 1);
                    std::vector<std::string> values = splitString(valuesStr, ',');
                    
                    // 匹配字段和值
                    for (size_t i = 0; i < fields.size() && i < values.size(); i++) {
                        std::string field = trim(fields[i]);
                        std::string value = trim(values[i]);
                        
                        // 移除引号
                        if (!value.empty() && (value[0] == '"' || value[0] == '\'')) {
                            value = value.substr(1, value.size() - 2);
                        }
                        
                        // 移除字段名的括号
                        if (!field.empty() && (field[0] == '`' || field[0] == '"' || field[0] == '\'')) {
                            field = field.substr(1, field.size() - 2);
                        }
                        
                        record[field] = value;
                    }
                }
            }
        }
        
        return record;
    }

    // 根据条件过滤记录
    void filterByCondition(const Table& table, const std::string& condition) {
        // 简单的条件过滤实现
        // 仅支持 field = value 格式的条件
        std::string::size_type pos = condition.find('=');
        if (pos != std::string::npos) {
            std::string field = trim(condition.substr(0, pos));
            std::string value = trim(condition.substr(pos + 1));
            
            // 移除值的引号
            if (!value.empty() && (value[0] == '"' || value[0] == '\'')) {
                value = value.substr(1, value.size() - 2);
            }
            
            // 移除字段名的括号
            if (!field.empty() && (field[0] == '`' || field[0] == '"' || field[0] == '\'')) {
                field = field.substr(1, field.size() - 2);
            }
            
            // 过滤记录
            for (const auto& record : table) {
                auto it = record.find(field);
                if (it != record.end() && it->second == value) {
                    currentTable_.push_back(record);
                }
            }
        } else {
            // 如果没有有效的条件，返回所有记录
            currentTable_ = table;
        }
    }

    // 分割字符串
    std::vector<std::string> splitString(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);
        
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        
        return tokens;
    }

    // 修剪字符串
    std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) {
            return "";
        }
        size_t end = str.find_last_not_of(" \t\n\r");
        return str.substr(start, end - start + 1);
    }

private:
    Table currentTable_;
    size_t currentRow_;
    long long lastInsertId_;
    bool isValid_;
};

// 创建内存数据库连接的工厂函数
DBConnectionPtr createMemoryConnection() {
    return std::make_shared<MemoryDBConnection>();
}

} // namespace models
