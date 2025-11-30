#ifndef RECORD_H
#define RECORD_H

#include <string>
#include <vector>
#include <memory>
#include <chrono>

namespace models {

// 记录状态枚举
enum class RecordStatus {
    AC,     // 通过
    PARTIAL,// 部分通过
    FAILED  // 失败
};

// 刷题记录实体类
class Record {
public:
    Record();
    Record(long long id, long long userId, long long problemId, 
           RecordStatus status, int timeSpentSeconds, const std::string& note);
    
    // Getters and setters
    long long getId() const;
    void setId(long long id);
    
    long long getUserId() const;
    void setUserId(long long userId);
    
    long long getProblemId() const;
    void setProblemId(long long problemId);
    
    RecordStatus getStatus() const;
    void setStatus(RecordStatus status);
    
    int getTimeSpentSeconds() const;
    void setTimeSpentSeconds(int timeSpentSeconds);
    
    const std::string& getNote() const;
    void setNote(const std::string& note);
    
    const std::chrono::system_clock::time_point& getCreatedAt() const;
    void setCreatedAt(const std::chrono::system_clock::time_point& createdAt);
    
    // 状态转换
    static RecordStatus stringToStatus(const std::string& status);
    static std::string statusToString(RecordStatus status);
    
    // 格式化时间为可读字符串
    std::string formatTimeSpent() const;

private:
    long long id_;
    long long userId_;
    long long problemId_;
    RecordStatus status_;
    int timeSpentSeconds_;
    std::string note_;
    std::chrono::system_clock::time_point createdAt_;
};

// 记录查询参数
struct RecordQueryParams {
    long long userId;
    long long problemId;
    RecordStatus status;
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;
    int page;
    int pageSize;
    
    RecordQueryParams() : userId(0), problemId(0), status(RecordStatus::AC), 
                        page(1), pageSize(10) {
    }
};

// 记录分页结果
struct RecordPageResult {
    std::vector<std::shared_ptr<Record>> records;
    int totalCount;
    int page;
    int pageSize;
    int totalPages;
};

// 用户统计信息
struct UserStatistics {
    int totalProblems;
    int passedProblems;
    int failedProblems;
    int partialProblems;
    
    // 按难度分类的统计
    struct DifficultyStats {
        int total;
        int passed;
        double passRate;
    };
    
    DifficultyStats easyStats;
    DifficultyStats mediumStats;
    DifficultyStats hardStats;
    
    // 最近30天刷题数量（日期和数量的映射）
    std::vector<std::pair<std::string, int>> last30DaysStats;
    
    UserStatistics() : totalProblems(0), passedProblems(0), 
                     failedProblems(0), partialProblems(0) {
        easyStats = {0, 0, 0.0};
        mediumStats = {0, 0, 0.0};
        hardStats = {0, 0, 0.0};
    }
};

// 刷题记录数据访问接口
class RecordRepository {
public:
    virtual ~RecordRepository() = default;
    
    // 创建记录
    virtual bool createRecord(const Record& record) = 0;
    
    // 根据ID获取记录
    virtual std::shared_ptr<Record> getRecordById(long long id) = 0;
    
    // 分页查询记录
    virtual RecordPageResult findRecords(const RecordQueryParams& params) = 0;
    
    // 获取用户统计信息
    virtual UserStatistics getUserStatistics(long long userId) = 0;
    
    // 获取用户最近30天的刷题统计
    virtual std::vector<std::pair<std::string, int>> getUserLast30DaysStats(long long userId) = 0;
    
    // 获取用户已完成的题目数量（按状态）
    virtual int getUserProblemCount(long long userId, RecordStatus status) = 0;
    
    // 获取用户按难度的完成情况
    virtual std::unordered_map<std::string, UserStatistics::DifficultyStats> getUserDifficultyStats(long long userId) = 0;
};

// 刷题记录数据访问实现（基于数据库）
class DatabaseRecordRepository : public RecordRepository {
public:
    DatabaseRecordRepository();
    ~DatabaseRecordRepository() override;
    
    bool createRecord(const Record& record) override;
    std::shared_ptr<Record> getRecordById(long long id) override;
    RecordPageResult findRecords(const RecordQueryParams& params) override;
    UserStatistics getUserStatistics(long long userId) override;
    std::vector<std::pair<std::string, int>> getUserLast30DaysStats(long long userId) override;
    int getUserProblemCount(long long userId, RecordStatus status) override;
    std::unordered_map<std::string, UserStatistics::DifficultyStats> getUserDifficultyStats(long long userId) override;
};

// 全局记录仓库实例
extern std::shared_ptr<RecordRepository> g_recordRepository;

// 初始化记录仓库
bool initRecordRepository();

} // namespace models

#endif // RECORD_H
