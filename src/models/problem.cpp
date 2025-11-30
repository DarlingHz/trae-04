#include "problem.h"
#include "db_pool.h"
#include "../common/logger.h"
#include "../common/error.h"
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace models {

Problem::Problem() : id_(0), difficulty_(ProblemDifficulty::EASY), 
                   createdAt_(std::chrono::system_clock::now()),
                   updatedAt_(std::chrono::system_clock::now()),
                   isDeleted_(false) {
}

Problem::Problem(long long id, const std::string& title, const std::string& description, 
                ProblemDifficulty difficulty, const std::vector<std::string>& tags)
    : id_(id), title_(title), description_(description), difficulty_(difficulty), 
      tags_(tags), createdAt_(std::chrono::system_clock::now()),
      updatedAt_(std::chrono::system_clock::now()), isDeleted_(false) {
}

long long Problem::getId() const {
    return id_;
}

void Problem::setId(long long id) {
    id_ = id;
}

const std::string& Problem::getTitle() const {
    return title_;
}

void Problem::setTitle(const std::string& title) {
    title_ = title;
    updatedAt_ = std::chrono::system_clock::now();
}

const std::string& Problem::getDescription() const {
    return description_;
}

void Problem::setDescription(const std::string& description) {
    description_ = description;
    updatedAt_ = std::chrono::system_clock::now();
}

ProblemDifficulty Problem::getDifficulty() const {
    return difficulty_;
}

void Problem::setDifficulty(ProblemDifficulty difficulty) {
    difficulty_ = difficulty;
    updatedAt_ = std::chrono::system_clock::now();
}

const std::vector<std::string>& Problem::getTags() const {
    return tags_;
}

void Problem::setTags(const std::vector<std::string>& tags) {
    tags_ = tags;
    updatedAt_ = std::chrono::system_clock::now();
}

const std::chrono::system_clock::time_point& Problem::getCreatedAt() const {
    return createdAt_;
}

void Problem::setCreatedAt(const std::chrono::system_clock::time_point& createdAt) {
    createdAt_ = createdAt;
}

const std::chrono::system_clock::time_point& Problem::getUpdatedAt() const {
    return updatedAt_;
}

void Problem::setUpdatedAt(const std::chrono::system_clock::time_point& updatedAt) {
    updatedAt_ = updatedAt;
}

bool Problem::isDeleted() const {
    return isDeleted_;
}

void Problem::setDeleted(bool deleted) {
    isDeleted_ = deleted;
    updatedAt_ = std::chrono::system_clock::now();
}

ProblemDifficulty Problem::stringToDifficulty(const std::string& difficulty) {
    if (difficulty == "EASY") {
        return ProblemDifficulty::EASY;
    } else if (difficulty == "MEDIUM") {
        return ProblemDifficulty::MEDIUM;
    } else if (difficulty == "HARD") {
        return ProblemDifficulty::HARD;
    }
    return ProblemDifficulty::MEDIUM; // 默认为中等难度
}

std::string Problem::difficultyToString(ProblemDifficulty difficulty) {
    switch (difficulty) {
        case ProblemDifficulty::EASY: return "EASY";
        case ProblemDifficulty::MEDIUM: return "MEDIUM";
        case ProblemDifficulty::HARD: return "HARD";
        default: return "MEDIUM";
    }
}

void Problem::addTag(const std::string& tag) {
    if (!hasTag(tag)) {
        tags_.push_back(tag);
        updatedAt_ = std::chrono::system_clock::now();
    }
}

bool Problem::removeTag(const std::string& tag) {
    auto it = std::find(tags_.begin(), tags_.end(), tag);
    if (it != tags_.end()) {
        tags_.erase(it);
        updatedAt_ = std::chrono::system_clock::now();
        return true;
    }
    return false;
}

bool Problem::hasTag(const std::string& tag) const {
    return std::find(tags_.begin(), tags_.end(), tag) != tags_.end();
}

std::string Problem::serializeTags() const {
    std::stringstream ss;
    for (size_t i = 0; i < tags_.size(); i++) {
        if (i > 0) {
            ss << ",";
        }
        ss << tags_[i];
    }
    return ss.str();
}

std::vector<std::string> Problem::deserializeTags(const std::string& tagsStr) {
    std::vector<std::string> tags;
    std::stringstream ss(tagsStr);
    std::string tag;
    
    while (std::getline(ss, tag, ',')) {
        if (!tag.empty()) {
            tags.push_back(tag);
        }
    }
    
    return tags;
}

DatabaseProblemRepository::DatabaseProblemRepository() {
}

DatabaseProblemRepository::~DatabaseProblemRepository() {
}

bool DatabaseProblemRepository::createProblem(const Problem& problem) {
    try {
        if (!g_dbPool) {
            throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Database pool not initialized");
        }
        
        // 获取数据库连接
        DBConnectionPtr conn = g_dbPool->getConnection();
        
        // 构建插入语句
        std::stringstream ss;
        ss << "INSERT INTO problems (title, description, difficulty, tags, created_at, updated_at, is_deleted) VALUES ('" 
           << problem.getTitle() << "', '" << problem.getDescription() << "', '" 
           << Problem::difficultyToString(problem.getDifficulty()) << "', '" 
           << problem.serializeTags() << "', CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, 0)";
        
        // 执行插入
        int result = conn->executeUpdate(ss.str());
        
        // 归还连接
        g_dbPool->releaseConnection(conn);
        
        if (result > 0) {
            common::g_logger.info("Problem created: %s", problem.getTitle().c_str());
            return true;
        } else {
            common::g_logger.error("Failed to create problem: %s", problem.getTitle().c_str());
            return false;
        }
    } catch (const common::AppException& e) {
        common::g_logger.error("Error creating problem: %s, code: %s", e.getMessage().c_str(), e.getCodeString().c_str());
        throw;
    } catch (const std::exception& e) {
        common::g_logger.error("Error creating problem: %s", e.what());
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, e.what());
    }
}

std::shared_ptr<Problem> DatabaseProblemRepository::getProblemById(long long id) {
    try {
        if (!g_dbPool) {
            throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Database pool not initialized");
        }
        
        // 获取数据库连接
        DBConnectionPtr conn = g_dbPool->getConnection();
        
        // 构建查询语句
        std::stringstream ss;
        ss << "SELECT id, title, description, difficulty, tags, created_at, updated_at, is_deleted " 
           << "FROM problems WHERE id = " << id << " AND is_deleted = 0";
        
        // 执行查询
        bool success = conn->executeQuery(ss.str());
        
        std::shared_ptr<Problem> problem;
        if (success && conn->hasNext()) {
            conn->next();
            
            ProblemDifficulty difficulty = Problem::stringToDifficulty(conn->getString("difficulty"));
            std::vector<std::string> tags = Problem::deserializeTags(conn->getString("tags"));
            
            problem = std::make_shared<Problem>(
                conn->getLongLong("id"),
                conn->getString("title"),
                conn->getString("description"),
                difficulty,
                tags
            );
            
            // 设置时间戳和删除状态
            problem->setDeleted(conn->getBool("is_deleted"));
        }
        
        // 归还连接
        g_dbPool->releaseConnection(conn);
        
        return problem;
    } catch (const common::AppException& e) {
        common::g_logger.error("Error getting problem by id %lld: %s, code: %s", id, e.getMessage().c_str(), e.getCodeString().c_str());
        throw;
    } catch (const std::exception& e) {
        common::g_logger.error("Error getting problem by id %lld: %s", id, e.what());
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, e.what());
    }
}

bool DatabaseProblemRepository::updateProblem(const Problem& problem) {
    try {
        if (!g_dbPool) {
            throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Database pool not initialized");
        }
        
        // 获取数据库连接
        DBConnectionPtr conn = g_dbPool->getConnection();
        
        // 构建更新语句
        std::stringstream ss;
        ss << "UPDATE problems SET title = '" << problem.getTitle() 
           << "', description = '" << problem.getDescription() 
           << "', difficulty = '" << Problem::difficultyToString(problem.getDifficulty()) 
           << "', tags = '" << problem.serializeTags() 
           << "', updated_at = CURRENT_TIMESTAMP "
           << "WHERE id = " << problem.getId() << " AND is_deleted = 0";
        
        // 执行更新
        int result = conn->executeUpdate(ss.str());
        
        // 归还连接
        g_dbPool->releaseConnection(conn);
        
        if (result > 0) {
            common::g_logger.info("Problem updated: %s", problem.getTitle().c_str());
            return true;
        } else {
            common::g_logger.error("Failed to update problem: %s", problem.getTitle().c_str());
            return false;
        }
    } catch (const common::AppException& e) {
        common::g_logger.error("Error updating problem: %s, code: %s", e.getMessage().c_str(), e.getCodeString().c_str());
        throw;
    } catch (const std::exception& e) {
        common::g_logger.error("Error updating problem: %s", e.what());
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, e.what());
    }
}

bool DatabaseProblemRepository::deleteProblem(long long id) {
    try {
        if (!g_dbPool) {
            throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Database pool not initialized");
        }
        
        // 获取数据库连接
        DBConnectionPtr conn = g_dbPool->getConnection();
        
        // 构建软删除语句
        std::stringstream ss;
        ss << "UPDATE problems SET is_deleted = 1, updated_at = CURRENT_TIMESTAMP WHERE id = " << id;
        
        // 执行更新
        int result = conn->executeUpdate(ss.str());
        
        // 归还连接
        g_dbPool->releaseConnection(conn);
        
        if (result > 0) {
            common::g_logger.info("Problem deleted: %lld", id);
            return true;
        } else {
            common::g_logger.error("Failed to delete problem: %lld", id);
            return false;
        }
    } catch (const common::AppException& e) {
        common::g_logger.error("Error deleting problem: %s, code: %s", e.getMessage().c_str(), e.getCodeString().c_str());
        throw;
    } catch (const std::exception& e) {
        common::g_logger.error("Error deleting problem: %s", e.what());
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, e.what());
    }
}

ProblemPageResult DatabaseProblemRepository::findProblems(const ProblemQueryParams& params) {
    ProblemPageResult result;
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
        whereClause << "WHERE is_deleted = 0";
        
        // 添加难度过滤
        std::string difficultyStr = Problem::difficultyToString(params.difficulty);
        if (difficultyStr != "MEDIUM") { // 如果不是默认值
            whereClause << " AND difficulty = '" << difficultyStr << "'";
        }
        
        // 添加关键字过滤
        if (!params.keyword.empty()) {
            whereClause << " AND title LIKE '%" << params.keyword << "%'";
        }
        
        // 添加标签过滤（简化实现，实际应该使用标签表）
        for (const auto& tag : params.tags) {
            whereClause << " AND tags LIKE '%" << tag << "%'";
        }
        
        // 查询总数
        std::stringstream countQuery;
        countQuery << "SELECT COUNT(*) as count FROM problems " << whereClause.str();
        conn->executeQuery(countQuery.str());
        if (conn->hasNext()) {
            conn->next();
            result.totalCount = conn->getInt("count");
        }
        
        // 计算总页数
        result.totalPages = (result.totalCount + params.pageSize - 1) / params.pageSize;
        
        // 构建分页查询
        std::stringstream query;
        query << "SELECT id, title, description, difficulty, tags, created_at, updated_at "
              << "FROM problems " << whereClause.str()
              << " ORDER BY created_at DESC LIMIT " << params.pageSize
              << " OFFSET " << (params.page - 1) * params.pageSize;
        
        // 执行查询
        bool success = conn->executeQuery(query.str());
        
        if (success) {
            while (conn->hasNext()) {
                conn->next();
                
                ProblemDifficulty difficulty = Problem::stringToDifficulty(conn->getString("difficulty"));
                std::vector<std::string> tags = Problem::deserializeTags(conn->getString("tags"));
                
                auto problem = std::make_shared<Problem>(
                    conn->getLongLong("id"),
                    conn->getString("title"),
                    conn->getString("description"),
                    difficulty,
                    tags
                );
                
                result.problems.push_back(problem);
            }
        }
        
        // 归还连接
        g_dbPool->releaseConnection(conn);
        
    } catch (const common::AppException& e) {
        common::g_logger.error("Error finding problems: %s, code: %s", e.getMessage().c_str(), e.getCodeString().c_str());
        throw;
    } catch (const std::exception& e) {
        common::g_logger.error("Error finding problems: %s", e.what());
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, e.what());
    }
    
    return result;
}

ProblemPageResult DatabaseProblemRepository::searchProblems(const std::string& keyword, 
                                                          const std::vector<std::string>& tags, 
                                                          ProblemDifficulty difficulty, 
                                                          int page, int pageSize) {
    ProblemQueryParams params;
    params.page = page;
    params.pageSize = pageSize;
    params.keyword = keyword;
    params.tags = tags;
    params.difficulty = difficulty;
    
    return findProblems(params);
}

// 全局题目仓库实例
std::shared_ptr<ProblemRepository> g_problemRepository;

bool initProblemRepository() {
    try {
        g_problemRepository = std::make_shared<DatabaseProblemRepository>();
        common::g_logger.info("Problem repository initialized successfully");
        return true;
    } catch (const std::exception& e) {
        common::g_logger.error("Failed to initialize problem repository: %s", e.what());
        return false;
    }
}

} // namespace models
