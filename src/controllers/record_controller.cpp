#include "record_controller.h"
#include "../common/logger.h"
#include "../models/record.h"
#include "../models/problem.h"
#include <chrono>
#include <sstream>

namespace controllers {

HttpResponse RecordController::handleRequest(const HttpRequest& request) {
    try {
        // 根据路径和方法分发请求
        std::string path = request.path;
        HttpMethod method = request.method;
        
        // 创建刷题记录接口
        if (path.find("/api/v1/problems/") == 0 && path.find("/records") != std::string::npos) {
            if (method == HttpMethod::POST) {
                return handleCreateRecord(request);
            }
        }
        // 查询用户刷题记录接口
        else if (path.find("/api/v1/users/") == 0 && path.find("/records") != std::string::npos) {
            if (method == HttpMethod::GET) {
                return handleGetUserRecords(request);
            }
        }
        // 获取用户统计信息接口
        else if (path.find("/api/v1/users/") == 0 && path.find("/stats") != std::string::npos) {
            if (method == HttpMethod::GET) {
                return handleGetUserStats(request);
            }
        }
        
        // 路径不匹配，返回404
        return createErrorResponse(common::ErrorCode::NOT_FOUND, "API endpoint not found");
    } catch (const common::AppException& e) {
        common::g_logger.error("Record controller error: %s, code: %s", e.getMessage().c_str(), e.getCodeString().c_str());
        return createErrorResponse(e);
    } catch (const std::exception& e) {
        common::g_logger.error("Record controller unexpected error: %s", e.what());
        return createErrorResponse(common::ErrorCode::INTERNAL_ERROR, "Internal server error");
    }
}

HttpResponse RecordController::handleCreateRecord(const HttpRequest& request) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 验证认证
    long long userId = authenticateRequest(request);
    
    // 提取题目ID
    long long problemId = extractProblemId(request);
    
    // 检查题目是否存在
    auto problem = models::g_problemRepository->getProblemById(problemId);
    if (!problem) {
        throw common::AppException(common::ErrorCode::NOT_FOUND, "Problem not found");
    }
    
    // 解析请求体
    common::JsonValue body = parseJson(request.body);
    
    // 验证必需字段
    if (!body.isObject() || !body["status"].isString() || body["status"].asString().empty()) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Status is required");
    }
    
    if (!body.isObject() || !body["time_spent_seconds"].isString() || body["time_spent_seconds"].asString().empty()) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Time spent seconds is required");
    }
    
    // 创建记录对象
    models::Record record;
    record.setUserId(userId);
    record.setProblemId(problemId);
    
    // 设置状态
    models::RecordStatus status = models::Record::stringToStatus(body["status"].asString());
    record.setStatus(status);
    
    // 设置用时
    try {
        int timeSpent = std::stoi(body["time_spent_seconds"].asString());
        if (timeSpent < 0) {
            throw common::AppException(common::ErrorCode::INVALID_PARAM, "Time spent seconds cannot be negative");
        }
        record.setTimeSpentSeconds(timeSpent);
    } catch (const std::invalid_argument&) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Invalid time spent seconds format");
    }
    
    // 设置备注（如果有）
    if (body.isObject() && body["note"].isString()) {
        record.setNote(body["note"].asString());
    }
    
    // 保存记录
    bool success = models::g_recordRepository->createRecord(record);
    if (!success) {
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Failed to create record");
    }
    
    // 构造响应
    common::JsonValue responseData;
    responseData["id"] = std::to_string(record.getId());
    responseData["user_id"] = std::to_string(record.getUserId());
    responseData["problem_id"] = std::to_string(record.getProblemId());
    responseData["status"] = models::Record::statusToString(record.getStatus());
    responseData["time_spent_seconds"] = std::to_string(record.getTimeSpentSeconds());
    responseData["time_spent_formatted"] = record.formatTimeSpent();
    responseData["note"] = record.getNote();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    common::g_logger.info("Record created: user_id=%lld, problem_id=%lld, status=%s, duration=%lldms", 
                     userId, problemId, models::Record::statusToString(status).c_str(), duration);
    
    return createSuccessResponse(responseData);
}

HttpResponse RecordController::handleGetUserRecords(const HttpRequest& request) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 验证认证
    long long currentUserId = authenticateRequest(request);
    
    // 提取目标用户ID
    long long targetUserId = extractUserId(request);
    
    // 检查权限：用户只能查看自己的记录
    if (currentUserId != targetUserId) {
        throw common::AppException(common::ErrorCode::FORBIDDEN, "You can only view your own records");
    }
    
    // 获取分页参数
    auto [page, pageSize] = getPaginationParams(request);
    
    // 构建查询参数
    models::RecordQueryParams params;
    params.userId = targetUserId;
    params.page = page;
    params.pageSize = pageSize;
    
    // 从查询参数获取过滤条件
    if (request.queryParams.find("problem_id") != request.queryParams.end()) {
        try {
            params.problemId = std::stoll(request.queryParams.at("problem_id"));
        } catch (...) {
            // 无效的题目ID，忽略该参数
        }
    }
    
    if (request.queryParams.find("status") != request.queryParams.end()) {
        params.status = models::Record::stringToStatus(request.queryParams.at("status"));
    }
    
    // 查询记录列表
    models::RecordPageResult result = models::g_recordRepository->findRecords(params);
    
    // 构造响应
    common::JsonValue responseData;
    responseData["page"] = std::to_string(result.page);
    responseData["page_size"] = std::to_string(result.pageSize);
    responseData["total_count"] = std::to_string(result.totalCount);
    responseData["total_pages"] = std::to_string(result.totalPages);
    
    // 构造记录列表JSON
    std::stringstream recordsArray;
    recordsArray << "[";
    
    bool first = true;
    for (const auto& record : result.records) {
        if (!first) {
            recordsArray << ",";
        }
        first = false;
        
        recordsArray << "{";
        recordsArray << '"' << "id" << '"' << ":" << '"' << record->getId() << '"' << ",";        
        recordsArray << '"' << "problem_id" << '"' << ":" << '"' << record->getProblemId() << '"' << ",";        
        recordsArray << '"' << "status" << '"' << ":" << '"' << models::Record::statusToString(record->getStatus()) << '"' << ",";        
        recordsArray << '"' << "time_spent_seconds" << '"' << ":" << '"' << record->getTimeSpentSeconds() << '"' << ",";        
        recordsArray << '"' << "time_spent_formatted" << '"' << ":" << '"' << record->formatTimeSpent() << '"' << ",";        
        recordsArray << '"' << "note" << '"' << ":" << '"' << record->getNote() << '"';
        recordsArray << '}';
    }
    
    recordsArray << "]";
    responseData["records"] = recordsArray.str();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    common::g_logger.info("User records retrieved: user_id=%lld, page=%d, page_size=%d, total=%d, duration=%lldms", 
                     targetUserId, page, pageSize, result.totalCount, duration);
    
    return createSuccessResponse(responseData);
}

HttpResponse RecordController::handleGetUserStats(const HttpRequest& request) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 验证认证
    long long currentUserId = authenticateRequest(request);
    
    // 提取目标用户ID
    long long targetUserId = extractUserId(request);
    
    // 检查权限：用户只能查看自己的统计
    if (currentUserId != targetUserId) {
        throw common::AppException(common::ErrorCode::FORBIDDEN, "You can only view your own statistics");
    }
    
    // 获取用户统计信息
    models::UserStatistics stats = models::g_recordRepository->getUserStatistics(targetUserId);
    
    // 构造响应
    common::JsonValue responseData;
    responseData["total_problems"] = std::to_string(stats.totalProblems);
    responseData["passed_problems"] = std::to_string(stats.passedProblems);
    responseData["partial_problems"] = std::to_string(stats.partialProblems);
    responseData["failed_problems"] = std::to_string(stats.failedProblems);
    
    // 计算总体通过率
    double overallPassRate = stats.totalProblems > 0 
        ? static_cast<double>(stats.passedProblems) / stats.totalProblems * 100 
        : 0.0;
    responseData["overall_pass_rate"] = std::to_string(overallPassRate);
    
    // 构造按难度分类的统计
    common::JsonValue difficultyStats;
    difficultyStats["easy_total"] = std::to_string(stats.easyStats.total);
    difficultyStats["easy_passed"] = std::to_string(stats.easyStats.passed);
    difficultyStats["easy_pass_rate"] = std::to_string(stats.easyStats.passRate);
    difficultyStats["medium_total"] = std::to_string(stats.mediumStats.total);
    difficultyStats["medium_passed"] = std::to_string(stats.mediumStats.passed);
    difficultyStats["medium_pass_rate"] = std::to_string(stats.mediumStats.passRate);
    difficultyStats["hard_total"] = std::to_string(stats.hardStats.total);
    difficultyStats["hard_passed"] = std::to_string(stats.hardStats.passed);
    difficultyStats["hard_pass_rate"] = std::to_string(stats.hardStats.passRate);
    
    responseData["difficulty_stats"] = toJson(difficultyStats);
    
    // 构造最近30天的统计
    std::stringstream last30DaysArray;
    last30DaysArray << "[";
    
    bool first = true;
    for (const auto& [date, count] : stats.last30DaysStats) {
        if (!first) {
            last30DaysArray << ",";
        }
        first = false;
        
        last30DaysArray << "{";
        last30DaysArray << '"' << "date" << '"' << ":" << '"' << date << '"' << ",";        
        last30DaysArray << '"' << "count" << '"' << ":" << '"' << count << '"';
        last30DaysArray << '}';
    }
    
    last30DaysArray << "]";
    responseData["last_30_days"] = last30DaysArray.str();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    common::g_logger.info("User statistics retrieved: user_id=%lld, duration=%lldms", targetUserId, duration);
    
    return createSuccessResponse(responseData);
}

long long RecordController::authenticateRequest(const HttpRequest& request) {
    // 从请求头获取token
    std::string token = request.getAuthToken();
    if (token.empty()) {
        throw common::AppException(common::ErrorCode::UNAUTHORIZED, "Authorization token is required");
    }
    
    // 验证token并获取用户信息
    auto user = auth::g_authService->validateToken(token);
    if (!user) {
        throw common::AppException(common::ErrorCode::USER_NOT_FOUND, "User not found");
    }
    
    return user->getId();
}

long long RecordController::extractProblemId(const HttpRequest& request) {
    std::string path = request.path;
    // 匹配 /api/v1/problems/{id}/records 格式
    size_t problemsPos = path.find("/api/v1/problems/");
    if (problemsPos == std::string::npos) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Invalid path format");
    }
    
    size_t startIdPos = problemsPos + 16; // "/api/v1/problems/" 的长度
    size_t endIdPos = path.find("/", startIdPos);
    
    if (endIdPos == std::string::npos || endIdPos <= startIdPos) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Problem ID not found in path");
    }
    
    try {
        std::string idStr = path.substr(startIdPos, endIdPos - startIdPos);
        return std::stoll(idStr);
    } catch (...) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Invalid problem ID");
    }
}

long long RecordController::extractUserId(const HttpRequest& request) {
    std::string path = request.path;
    // 匹配 /api/v1/users/{id}/records 或 /api/v1/users/{id}/stats 格式
    size_t usersPos = path.find("/api/v1/users/");
    if (usersPos == std::string::npos) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Invalid path format");
    }
    
    size_t startIdPos = usersPos + 14; // "/api/v1/users/" 的长度
    size_t endIdPos = path.find("/", startIdPos);
    
    if (endIdPos == std::string::npos || endIdPos <= startIdPos) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "User ID not found in path");
    }
    
    try {
        std::string idStr = path.substr(startIdPos, endIdPos - startIdPos);
        return std::stoll(idStr);
    } catch (...) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Invalid user ID");
    }
}

} // namespace controllers
