#include "record.h"
#include "db_pool.h"
#include "../common/logger.h"
#include "../common/error.h"
#include <sstream>
#include <iomanip>
#include <unordered_map>

namespace models {

Record::Record() : id_(0), userId_(0), problemId_(0), status_(RecordStatus::FAILED), 
                 timeSpentSeconds_(0), createdAt_(std::chrono::system_clock::now()) {
}

Record::Record(long long id, long long userId, long long problemId, 
              RecordStatus status, int timeSpentSeconds, const std::string& note)
    : id_(id), userId_(userId), problemId_(problemId), status_(status), 
      timeSpentSeconds_(timeSpentSeconds), note_(note), 
      createdAt_(std::chrono::system_clock::now()) {
}

long long Record::getId() const {
    return id_;
}

void Record::setId(long long id) {
    id_ = id;
}

long long Record::getUserId() const {
    return userId_;
}

void Record::setUserId(long long userId) {
    userId_ = userId;
}

long long Record::getProblemId() const {
    return problemId_;
}

void Record::setProblemId(long long problemId) {
    problemId_ = problemId;
}

RecordStatus Record::getStatus() const {
    return status_;
}

void Record::setStatus(RecordStatus status) {
    status_ = status;
}

int Record::getTimeSpentSeconds() const {
    return timeSpentSeconds_;
}

void Record::setTimeSpentSeconds(int timeSpentSeconds) {
    timeSpentSeconds_ = timeSpentSeconds;
}

const std::string& Record::getNote() const {
    return note_;
}

void Record::setNote(const std::string& note) {
    note_ = note;
}

const std::chrono::system_clock::time_point& Record::getCreatedAt() const {
    return createdAt_;
}

void Record::setCreatedAt(const std::chrono::system_clock::time_point& createdAt) {
    createdAt_ = createdAt;
}

RecordStatus Record::stringToStatus(const std::string& status) {
    if (status == "AC") {
        return RecordStatus::AC;
    } else if (status == "PARTIAL") {
        return RecordStatus::PARTIAL;
    } else if (status == "FAILED") {
        return RecordStatus::FAILED;
    }
    return RecordStatus::FAILED; // 默认为失败
}

std::string Record::statusToString(RecordStatus status) {
    switch (status) {
        case RecordStatus::AC: return "AC";
        case RecordStatus::PARTIAL: return "PARTIAL";
        case RecordStatus::FAILED: return "FAILED";
        default: return "FAILED";
    }
}

std::string Record::formatTimeSpent() const {
    int hours = timeSpentSeconds_ / 3600;
    int minutes = (timeSpentSeconds_ % 3600) / 60;
    int seconds = timeSpentSeconds_ % 60;
    
    std::stringstream ss;
    if (hours > 0) {
        ss << hours << "h " << minutes << "m " << seconds << "s";
    } else if (minutes > 0) {
        ss << minutes << "m " << seconds << "s";
    } else {
        ss << seconds << "s";
    }
    
    return ss.str();
}

DatabaseRecordRepository::DatabaseRecordRepository() {
}

DatabaseRecordRepository::~DatabaseRecordRepository() {
}

bool DatabaseRecordRepository::createRecord(const Record& record) {
    try {
        if (!g_dbPool) {
            throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Database pool not initialized");
        }
        
        // 获取数据库连接
        DBConnectionPtr conn = g_dbPool->getConnection();
        
        // 构建插入语句
        std::stringstream ss;
        ss << "INSERT INTO records (user_id, problem_id, status, time_spent_seconds, note, created_at) VALUES (" 
           << record.getUserId() << ", " << record.getProblemId() << ", '" 
           << Record::statusToString(record.getStatus()) << "', " << record.getTimeSpentSeconds() << ", '" 
           << record.getNote() << "', CURRENT_TIMESTAMP)";
        
        // 执行插入
        int result = conn->executeUpdate(ss.str());
        
        // 归还连接
        g_dbPool->releaseConnection(conn);
        
        if (result > 0) {
            common::g_logger.info("Record created: user_id=%lld, problem_id=%lld, status=%s", 
                              record.getUserId(), record.getProblemId(), 
                              Record::statusToString(record.getStatus()).c_str());
            return true;
        } else {
            common::g_logger.error("Failed to create record: user_id=%lld, problem_id=%lld", 
                               record.getUserId(), record.getProblemId());
            return false;
        }
    } catch (const common::AppException& e) {
        common::g_logger.error("Error creating record: %s, code: %s", e.getMessage().c_str(), e.getCodeString().c_str());
        throw;
    } catch (const std::exception& e) {
        common::g_logger.error("Error creating record: %s", e.what());
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, e.what());
    }
}

std::shared_ptr<Record> DatabaseRecordRepository::getRecordById(long long id) {
    try {
        if (!g_dbPool) {
            throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Database pool not initialized");
        }
        
        // 获取数据库连接
        DBConnectionPtr conn = g_dbPool->getConnection();
        
        // 构建查询语句
        std::stringstream ss;
        ss << "SELECT id, user_id, problem_id, status, time_spent_seconds, note, created_at " 
           << "FROM records WHERE id = " << id;
        
        // 执行查询
        bool success = conn->executeQuery(ss.str());
        
        std::shared_ptr<Record> record;
        if (success && conn->hasNext()) {
            conn->next();
            
            RecordStatus status = Record::stringToStatus(conn->getString("status"));
            
            record = std::make_shared<Record>(
                conn->getLongLong("id"),
                conn->getLongLong("user_id"),
                conn->getLongLong("problem_id"),
                status,
                conn->getInt("time_spent_seconds"),
                conn->getString("note")
            );
        }
        
        // 归还连接
        g_dbPool->releaseConnection(conn);
        
        return record;
    } catch (const common::AppException& e) {
        common::g_logger.error("Error getting record by id %lld: %s, code: %s", id, e.getMessage().c_str(), e.getCodeString().c_str());
        throw;
    } catch (const std::exception& e) {
        common::g_logger.error("Error getting record by id %lld: %s", id, e.what());
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, e.what());
    }
}

RecordPageResult DatabaseRecordRepository::findRecords(const RecordQueryParams& params) {
    RecordPageResult result;
    result.page = params.page;
    result.pageSize = params.pageSize;
    
    try {
        if (!g_dbPool) {
            throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Database pool not initialized");
        }
        
        // 获取数据库连接
        DBConnectionPtr conn = g_dbPool->getConnection();
        
        // 构建查询条件
        std::stringstream whereClause;
        whereClause << "WHERE 1=1";
        
        if (params.userId > 0) {
            whereClause << " AND user_id = " << params.userId;
        }
        
        if (params.problemId > 0) {
            whereClause << " AND problem_id = " << params.problemId;
        }
        
        std::string statusStr = Record::statusToString(params.status);
        if (statusStr != "FAILED") { // 如果不是默认值
            whereClause << " AND status = '" << statusStr << "'";
        }
        
        // 添加时间范围过滤
        // 注意：这里简化处理，实际应该使用正确的时间格式转换
        
        // 查询总数
        std::stringstream countQuery;
        countQuery << "SELECT COUNT(*) as count FROM records " << whereClause.str();
        conn->executeQuery(countQuery.str());
        if (conn->hasNext()) {
            conn->next();
            result.totalCount = conn->getInt("count");
        }
        
        // 计算总页数
        result.totalPages = (result.totalCount + params.pageSize - 1) / params.pageSize;
        
        // 构建分页查询
        std::stringstream query;
        query << "SELECT id, user_id, problem_id, status, time_spent_seconds, note, created_at "
              << "FROM records " << whereClause.str()
              << " ORDER BY created_at DESC LIMIT " << params.pageSize
              << " OFFSET " << (params.page - 1) * params.pageSize;
        
        // 执行查询
        bool success = conn->executeQuery(query.str());
        
        if (success) {
            while (conn->hasNext()) {
                conn->next();
                
                RecordStatus status = Record::stringToStatus(conn->getString("status"));
                
                auto record = std::make_shared<Record>(
                    conn->getLongLong("id"),
                    conn->getLongLong("user_id"),
                    conn->getLongLong("problem_id"),
                    status,
                    conn->getInt("time_spent_seconds"),
                    conn->getString("note")
                );
                
                result.records.push_back(record);
            }
        }
        
        // 归还连接
        g_dbPool->releaseConnection(conn);
        
    } catch (const common::AppException& e) {
        common::g_logger.error("Error finding records: %s, code: %s", e.getMessage().c_str(), e.getCodeString().c_str());
        throw;
    } catch (const std::exception& e) {
        common::g_logger.error("Error finding records: %s", e.what());
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, e.what());
    }
    
    return result;
}

UserStatistics DatabaseRecordRepository::getUserStatistics(long long userId) {
    UserStatistics stats;
    
    try {
        if (!g_dbPool) {
            throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Database pool not initialized");
        }
        
        // 获取数据库连接
        DBConnectionPtr conn = g_dbPool->getConnection();
        
        // 查询总刷题数
        std::stringstream totalQuery;
        totalQuery << "SELECT COUNT(*) as count FROM records WHERE user_id = " << userId;
        conn->executeQuery(totalQuery.str());
        if (conn->hasNext()) {
            conn->next();
            stats.totalProblems = conn->getInt("count");
        }
        
        // 查询通过的题目数
        std::stringstream acQuery;
        acQuery << "SELECT COUNT(*) as count FROM records WHERE user_id = " << userId 
                << " AND status = 'AC'";
        conn->executeQuery(acQuery.str());
        if (conn->hasNext()) {
            conn->next();
            stats.passedProblems = conn->getInt("count");
        }
        
        // 查询部分通过的题目数
        std::stringstream partialQuery;
        partialQuery << "SELECT COUNT(*) as count FROM records WHERE user_id = " << userId 
                    << " AND status = 'PARTIAL'";
        conn->executeQuery(partialQuery.str());
        if (conn->hasNext()) {
            conn->next();
            stats.partialProblems = conn->getInt("count");
        }
        
        // 查询失败的题目数
        std::stringstream failedQuery;
        failedQuery << "SELECT COUNT(*) as count FROM records WHERE user_id = " << userId 
                   << " AND status = 'FAILED'";
        conn->executeQuery(failedQuery.str());
        if (conn->hasNext()) {
            conn->next();
            stats.failedProblems = conn->getInt("count");
        }
        
        // 获取按难度分类的统计
        auto difficultyStats = getUserDifficultyStats(userId);
        if (difficultyStats.find("EASY") != difficultyStats.end()) {
            stats.easyStats = difficultyStats["EASY"];
        }
        if (difficultyStats.find("MEDIUM") != difficultyStats.end()) {
            stats.mediumStats = difficultyStats["MEDIUM"];
        }
        if (difficultyStats.find("HARD") != difficultyStats.end()) {
            stats.hardStats = difficultyStats["HARD"];
        }
        
        // 获取最近30天的统计
        stats.last30DaysStats = getUserLast30DaysStats(userId);
        
        // 归还连接
        g_dbPool->releaseConnection(conn);
        
    } catch (const common::AppException& e) {
        common::g_logger.error("Error getting user statistics: %s, code: %s", e.getMessage().c_str(), e.getCodeString().c_str());
        throw;
    } catch (const std::exception& e) {
        common::g_logger.error("Error getting user statistics: %s", e.what());
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, e.what());
    }
    
    return stats;
}

std::vector<std::pair<std::string, int>> DatabaseRecordRepository::getUserLast30DaysStats(long long userId) {
    std::vector<std::pair<std::string, int>> stats;
    
    try {
        if (!g_dbPool) {
            throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Database pool not initialized");
        }
        
        // 获取数据库连接
        DBConnectionPtr conn = g_dbPool->getConnection();
        
        // 查询最近30天每天的刷题数
        // 注意：这里简化处理，实际应该使用正确的日期函数
        std::stringstream query;
        query << "SELECT DATE(created_at) as date, COUNT(*) as count "
              << "FROM records WHERE user_id = " << userId 
              << " AND created_at >= DATE_SUB(CURRENT_DATE, INTERVAL 30 DAY)"
              << " GROUP BY DATE(created_at) ORDER BY date";
        
        bool success = conn->executeQuery(query.str());
        
        if (success) {
            while (conn->hasNext()) {
                conn->next();
                stats.push_back(std::make_pair(
                    conn->getString("date"),
                    conn->getInt("count")
                ));
            }
        }
        
        // 归还连接
        g_dbPool->releaseConnection(conn);
        
    } catch (const common::AppException& e) {
        common::g_logger.error("Error getting user last 30 days stats: %s, code: %s", e.getMessage().c_str(), e.getCodeString().c_str());
        throw;
    } catch (const std::exception& e) {
        common::g_logger.error("Error getting user last 30 days stats: %s", e.what());
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, e.what());
    }
    
    return stats;
}

int DatabaseRecordRepository::getUserProblemCount(long long userId, RecordStatus status) {
    try {
        if (!g_dbPool) {
            throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Database pool not initialized");
        }
        
        // 获取数据库连接
        DBConnectionPtr conn = g_dbPool->getConnection();
        
        // 构建查询语句
        std::stringstream ss;
        ss << "SELECT COUNT(*) as count FROM records WHERE user_id = " << userId 
           << " AND status = '" << Record::statusToString(status) << "'";
        
        // 执行查询
        conn->executeQuery(ss.str());
        
        int count = 0;
        if (conn->hasNext()) {
            conn->next();
            count = conn->getInt("count");
        }
        
        // 归还连接
        g_dbPool->releaseConnection(conn);
        
        return count;
    } catch (const common::AppException& e) {
        common::g_logger.error("Error getting user problem count: %s, code: %s", e.getMessage().c_str(), e.getCodeString().c_str());
        throw;
    } catch (const std::exception& e) {
        common::g_logger.error("Error getting user problem count: %s", e.what());
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, e.what());
    }
}

std::unordered_map<std::string, UserStatistics::DifficultyStats> DatabaseRecordRepository::getUserDifficultyStats(long long userId) {
    std::unordered_map<std::string, UserStatistics::DifficultyStats> stats;
    
    try {
        if (!g_dbPool) {
            throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Database pool not initialized");
        }
        
        // 获取数据库连接
        DBConnectionPtr conn = g_dbPool->getConnection();
        
        // 查询按难度分类的统计（简化实现，实际应该联表查询）
        // 这里我们假设每个难度的统计
        std::vector<std::string> difficulties = {"EASY", "MEDIUM", "HARD"};
        
        for (const auto& difficulty : difficulties) {
            UserStatistics::DifficultyStats difficultyStats;
            
            // 查询该难度的总题目数
            std::stringstream totalQuery;
            totalQuery << "SELECT COUNT(*) as count FROM records r, problems p "
                      << "WHERE r.user_id = " << userId 
                      << " AND r.problem_id = p.id AND p.difficulty = '" << difficulty << "'";
            
            conn->executeQuery(totalQuery.str());
            if (conn->hasNext()) {
                conn->next();
                difficultyStats.total = conn->getInt("count");
            }
            
            // 查询该难度通过的题目数
            std::stringstream passedQuery;
            passedQuery << "SELECT COUNT(*) as count FROM records r, problems p "
                       << "WHERE r.user_id = " << userId 
                       << " AND r.problem_id = p.id AND p.difficulty = '" << difficulty << "'"
                       << " AND r.status = 'AC'";
            
            conn->executeQuery(passedQuery.str());
            if (conn->hasNext()) {
                conn->next();
                difficultyStats.passed = conn->getInt("count");
            }
            
            // 计算通过率
            difficultyStats.passRate = difficultyStats.total > 0 
                ? static_cast<double>(difficultyStats.passed) / difficultyStats.total * 100 
                : 0.0;
            
            stats[difficulty] = difficultyStats;
        }
        
        // 归还连接
        g_dbPool->releaseConnection(conn);
        
    } catch (const common::AppException& e) {
        common::g_logger.error("Error getting user difficulty stats: %s, code: %s", e.getMessage().c_str(), e.getCodeString().c_str());
        throw;
    } catch (const std::exception& e) {
        common::g_logger.error("Error getting user difficulty stats: %s", e.what());
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, e.what());
    }
    
    return stats;
}

// 全局记录仓库实例
std::shared_ptr<RecordRepository> g_recordRepository;

bool initRecordRepository() {
    try {
        g_recordRepository = std::make_shared<DatabaseRecordRepository>();
        common::g_logger.info("Record repository initialized successfully");
        return true;
    } catch (const std::exception& e) {
        common::g_logger.error("Failed to initialize record repository: %s", e.what());
        return false;
    }
}

} // namespace models
