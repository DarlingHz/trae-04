#ifndef PROBLEM_H
#define PROBLEM_H

#include <string>
#include <vector>
#include <memory>
#include <chrono>

namespace models {

// 题目难度枚举
enum class ProblemDifficulty {
    EASY,
    MEDIUM,
    HARD
};

// 题目实体类
class Problem {
public:
    Problem();
    Problem(long long id, const std::string& title, const std::string& description, 
            ProblemDifficulty difficulty, const std::vector<std::string>& tags);
    
    // Getters and setters
    long long getId() const;
    void setId(long long id);
    
    const std::string& getTitle() const;
    void setTitle(const std::string& title);
    
    const std::string& getDescription() const;
    void setDescription(const std::string& description);
    
    ProblemDifficulty getDifficulty() const;
    void setDifficulty(ProblemDifficulty difficulty);
    
    const std::vector<std::string>& getTags() const;
    void setTags(const std::vector<std::string>& tags);
    
    const std::chrono::system_clock::time_point& getCreatedAt() const;
    void setCreatedAt(const std::chrono::system_clock::time_point& createdAt);
    
    const std::chrono::system_clock::time_point& getUpdatedAt() const;
    void setUpdatedAt(const std::chrono::system_clock::time_point& updatedAt);
    
    bool isDeleted() const;
    void setDeleted(bool deleted);
    
    // 难度转换
    static ProblemDifficulty stringToDifficulty(const std::string& difficulty);
    static std::string difficultyToString(ProblemDifficulty difficulty);
    
    // 标签操作
    void addTag(const std::string& tag);
    bool removeTag(const std::string& tag);
    bool hasTag(const std::string& tag) const;
    
    // 序列化标签为字符串
    std::string serializeTags() const;
    
    // 从字符串反序列化标签
    static std::vector<std::string> deserializeTags(const std::string& tagsStr);

private:
    long long id_;
    std::string title_;
    std::string description_;
    ProblemDifficulty difficulty_;
    std::vector<std::string> tags_;
    std::chrono::system_clock::time_point createdAt_;
    std::chrono::system_clock::time_point updatedAt_;
    bool isDeleted_;
};

// 题目查询参数
struct ProblemQueryParams {
    int page;
    int pageSize;
    ProblemDifficulty difficulty;
    std::vector<std::string> tags;
    std::string keyword;
    
    ProblemQueryParams() : page(1), pageSize(10), difficulty(ProblemDifficulty::EASY) {
    }
};

// 题目分页结果
struct ProblemPageResult {
    std::vector<std::shared_ptr<Problem>> problems;
    int totalCount;
    int page;
    int pageSize;
    int totalPages;
};

// 题目数据访问接口
class ProblemRepository {
public:
    virtual ~ProblemRepository() = default;
    
    // 创建题目
    virtual bool createProblem(const Problem& problem) = 0;
    
    // 根据ID获取题目
    virtual std::shared_ptr<Problem> getProblemById(long long id) = 0;
    
    // 更新题目
    virtual bool updateProblem(const Problem& problem) = 0;
    
    // 删除题目（软删除）
    virtual bool deleteProblem(long long id) = 0;
    
    // 分页查询题目
    virtual ProblemPageResult findProblems(const ProblemQueryParams& params) = 0;
    
    // 搜索题目
    virtual ProblemPageResult searchProblems(const std::string& keyword, 
                                           const std::vector<std::string>& tags, 
                                           ProblemDifficulty difficulty, 
                                           int page, int pageSize) = 0;
};

// 题目数据访问实现（基于数据库）
class DatabaseProblemRepository : public ProblemRepository {
public:
    DatabaseProblemRepository();
    ~DatabaseProblemRepository() override;
    
    bool createProblem(const Problem& problem) override;
    std::shared_ptr<Problem> getProblemById(long long id) override;
    bool updateProblem(const Problem& problem) override;
    bool deleteProblem(long long id) override;
    ProblemPageResult findProblems(const ProblemQueryParams& params) override;
    ProblemPageResult searchProblems(const std::string& keyword, 
                                   const std::vector<std::string>& tags, 
                                   ProblemDifficulty difficulty, 
                                   int page, int pageSize) override;
};

// 全局题目仓库实例
extern std::shared_ptr<ProblemRepository> g_problemRepository;

// 初始化题目仓库
bool initProblemRepository();

} // namespace models

#endif // PROBLEM_H
