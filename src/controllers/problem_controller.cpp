#include "problem_controller.h"
#include "../common/logger.h"
#include "../models/problem.h"
#include <chrono>
#include <sstream>

namespace controllers {

HttpResponse ProblemController::handleRequest(const HttpRequest& request) {
    try {
        // 根据路径和方法分发请求
        std::string path = request.path;
        HttpMethod method = request.method;
        
        // 题目详情相关接口
        if (path.find("/api/v1/problems/") == 0) {
            size_t lastSlash = path.rfind('/');
            if (lastSlash != std::string::npos && lastSlash + 1 < path.size()) {
                std::string idPart = path.substr(lastSlash + 1);
                // 检查是否是数字ID
                bool isNumber = !idPart.empty();
                for (char c : idPart) {
                    if (!std::isdigit(c)) {
                        isNumber = false;
                        break;
                    }
                }
                
                if (isNumber) {
                    // 是题目ID，根据HTTP方法处理
                    if (method == HttpMethod::GET) {
                        return handleGetProblem(request);
                    } else if (method == HttpMethod::PUT) {
                        return handleUpdateProblem(request);
                    } else if (method == HttpMethod::DELETE) {
                        return handleDeleteProblem(request);
                    }
                }
            }
        } 
        // 问题列表接口
        else if (path == "/api/v1/problems" && method == HttpMethod::GET) {
            return handleListProblems(request);
        }
        // 创建题目接口
        else if (path == "/api/v1/problems" && method == HttpMethod::POST) {
            return handleCreateProblem(request);
        }
        // 搜索接口
        else if (path == "/api/v1/problems/search" && method == HttpMethod::GET) {
            return handleSearchProblems(request);
        }
        
        // 路径不匹配，返回404
        return createErrorResponse(common::ErrorCode::NOT_FOUND, "API endpoint not found");
    } catch (const common::AppException& e) {
        common::g_logger.error("Problem controller error: %s, code: %s", e.getMessage().c_str(), e.getCodeString().c_str());
        return createErrorResponse(e);
    } catch (const std::exception& e) {
        common::g_logger.error("Problem controller unexpected error: %s", e.what());
        return createErrorResponse(common::ErrorCode::INTERNAL_ERROR, "Internal server error");
    }
}

HttpResponse ProblemController::handleCreateProblem(const HttpRequest& request) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 验证认证
    long long userId = authenticateRequest(request);
    
    // 解析请求体
    common::JsonValue body = parseJson(request.body);
    
    // 验证必需字段
    if (!body.isObject() || !body["title"].isString() || body["title"].asString().empty()) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Title is required");
    }
    if (!body.isObject() || !body["description"].isString() || body["description"].asString().empty()) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Description is required");
    }
    if (!body.isObject() || !body["difficulty"].isString() || body["difficulty"].asString().empty()) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Difficulty is required");
    }
    
    // 创建题目对象
    models::Problem problem;
    problem.setTitle(body["title"].asString());
    problem.setDescription(body["description"].asString());
    
    // 设置难度
    models::ProblemDifficulty difficulty = models::Problem::stringToDifficulty(body["difficulty"].asString());
    problem.setDifficulty(difficulty);
    
    // 设置标签（如果有）
    if (body.isObject() && body["tags"].isString()) {
        std::string tagsStr = body["tags"].asString();
        // 这里简化处理，实际应该解析更复杂的格式
        problem.setTags(models::Problem::deserializeTags(tagsStr));
    }
    
    // 保存题目
    bool success = models::g_problemRepository->createProblem(problem);
    if (!success) {
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Failed to create problem");
    }
    
    // 构造响应
    common::JsonValue responseData;
    responseData["id"] = std::to_string(problem.getId());
    responseData["title"] = problem.getTitle();
    responseData["description"] = problem.getDescription();
    responseData["difficulty"] = models::Problem::difficultyToString(problem.getDifficulty());
    responseData["tags"] = problem.serializeTags();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    common::g_logger.info("Problem created: id=%lld, title=%s, created_by=%lld, duration=%lldms", 
                     problem.getId(), problem.getTitle().c_str(), userId, duration);
    
    return createSuccessResponse(responseData);
}

HttpResponse ProblemController::handleGetProblem(const HttpRequest& request) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 提取题目ID
    long long problemId = extractProblemId(request);
    
    // 获取题目详情
    auto problem = models::g_problemRepository->getProblemById(problemId);
    if (!problem) {
        throw common::AppException(common::ErrorCode::NOT_FOUND, "Problem not found");
    }
    
    // 构造响应
    common::JsonValue responseData;
    responseData["id"] = std::to_string(problem->getId());
    responseData["title"] = problem->getTitle();
    responseData["description"] = problem->getDescription();
    responseData["difficulty"] = models::Problem::difficultyToString(problem->getDifficulty());
    responseData["tags"] = problem->serializeTags();
    // 格式化时间为字符串
    auto formatTime = [](const std::chrono::system_clock::time_point& time) {
        std::time_t timeT = std::chrono::system_clock::to_time_t(time);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    };
    responseData["created_at"] = formatTime(problem->getCreatedAt());
    responseData["updated_at"] = formatTime(problem->getUpdatedAt());
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    common::g_logger.info("Problem retrieved: id=%lld, title=%s, duration=%lldms", 
                     problem->getId(), problem->getTitle().c_str(), duration);
    
    return createSuccessResponse(responseData);
}

HttpResponse ProblemController::handleUpdateProblem(const HttpRequest& request) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 验证认证
    long long userId = authenticateRequest(request);
    
    // 提取题目ID
    long long problemId = extractProblemId(request);
    
    // 获取题目详情
    auto problem = models::g_problemRepository->getProblemById(problemId);
    if (!problem) {
        throw common::AppException(common::ErrorCode::NOT_FOUND, "Problem not found");
    }
    
    // 解析请求体
    common::JsonValue body = parseJson(request.body);
    
    // 更新字段
    if (body.isObject() && body["title"].isString()) {
        problem->setTitle(body["title"].asString());
    }
    if (body.isObject() && body["description"].isString()) {
        problem->setDescription(body["description"].asString());
    }
    if (body.isObject() && body["difficulty"].isString()) {
        models::ProblemDifficulty difficulty = models::Problem::stringToDifficulty(body["difficulty"].asString());
        problem->setDifficulty(difficulty);
    }
    if (body.isObject() && body["tags"].isString()) {
        problem->setTags(models::Problem::deserializeTags(body["tags"].asString()));
    }
    
    // 保存更新
    bool success = models::g_problemRepository->updateProblem(*problem);
    if (!success) {
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Failed to update problem");
    }
    
    // 构造响应
    common::JsonValue responseData;
    responseData["id"] = std::to_string(problem->getId());
    responseData["title"] = problem->getTitle();
    responseData["description"] = problem->getDescription();
    responseData["difficulty"] = models::Problem::difficultyToString(problem->getDifficulty());
    responseData["tags"] = problem->serializeTags();
    // 格式化时间为字符串
    auto formatTime = [](const std::chrono::system_clock::time_point& time) {
        std::time_t timeT = std::chrono::system_clock::to_time_t(time);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    };
    responseData["updated_at"] = formatTime(problem->getUpdatedAt());
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    common::g_logger.info("Problem updated: id=%lld, title=%s, updated_by=%lld, duration=%lldms", 
                     problem->getId(), problem->getTitle().c_str(), userId, duration);
    
    return createSuccessResponse(responseData);
}

HttpResponse ProblemController::handleDeleteProblem(const HttpRequest& request) {
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
    
    // 执行软删除
    bool success = models::g_problemRepository->deleteProblem(problemId);
    if (!success) {
        throw common::AppException(common::ErrorCode::DATABASE_ERROR, "Failed to delete problem");
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    common::g_logger.info("Problem deleted: id=%lld, title=%s, deleted_by=%lld, duration=%lldms", 
                     problem->getId(), problem->getTitle().c_str(), userId, duration);
    
    // 构造响应
    common::JsonValue responseData;
    responseData["message"] = "Problem deleted successfully";
    
    return createSuccessResponse(responseData);
}

HttpResponse ProblemController::handleListProblems(const HttpRequest& request) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 获取分页参数
    auto [page, pageSize] = getPaginationParams(request);
    
    // 构建查询参数
    models::ProblemQueryParams params;
    params.page = page;
    params.pageSize = pageSize;
    
    // 从查询参数获取过滤条件
    if (request.queryParams.find("difficulty") != request.queryParams.end()) {
        params.difficulty = models::Problem::stringToDifficulty(request.queryParams.at("difficulty"));
    }
    
    if (request.queryParams.find("tags") != request.queryParams.end()) {
        params.tags = models::Problem::deserializeTags(request.queryParams.at("tags"));
    }
    
    if (request.queryParams.find("keyword") != request.queryParams.end()) {
        params.keyword = request.queryParams.at("keyword");
    }
    
    // 查询题目列表
    models::ProblemPageResult result = models::g_problemRepository->findProblems(params);
    
    // 构造响应
    common::JsonValue responseData;
    responseData["page"] = std::to_string(result.page);
    responseData["page_size"] = std::to_string(result.pageSize);
    responseData["total_count"] = std::to_string(result.totalCount);
    responseData["total_pages"] = std::to_string(result.totalPages);
    
    // 构造题目列表JSON
    std::stringstream problemsArray;
    problemsArray << "[";
    
    bool first = true;
    for (const auto& problem : result.problems) {
        if (!first) {
            problemsArray << ",";
        }
        first = false;
        
        problemsArray << "{";
        problemsArray << '"' << "id" << '"' << ":" << '"' << problem->getId() << '"' << ",";
        problemsArray << '"' << "title" << '"' << ":" << '"' << problem->getTitle() << '"' << ",";
        problemsArray << '"' << "difficulty" << '"' << ":" << '"' << 
            models::Problem::difficultyToString(problem->getDifficulty()) << '"' << ",";
        problemsArray << '"' << "tags" << '"' << ":" << '"' << problem->serializeTags() << '"';
        problemsArray << '}';
    }
    
    problemsArray << "]";
    responseData["problems"] = problemsArray.str();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    common::g_logger.info("Problem list retrieved: page=%d, page_size=%d, total=%d, duration=%lldms", 
                     page, pageSize, result.totalCount, duration);
    
    return createSuccessResponse(responseData);
}

HttpResponse ProblemController::handleSearchProblems(const HttpRequest& request) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 获取分页参数
    auto [page, pageSize] = getPaginationParams(request);
    
    // 构建查询参数
    models::ProblemQueryParams params;
    params.page = page;
    params.pageSize = pageSize;
    
    // 从查询参数获取搜索条件
    if (request.queryParams.find("keyword") != request.queryParams.end()) {
        params.keyword = request.queryParams.at("keyword");
    }
    
    if (request.queryParams.find("tags") != request.queryParams.end()) {
        params.tags = models::Problem::deserializeTags(request.queryParams.at("tags"));
    }
    
    if (request.queryParams.find("difficulty") != request.queryParams.end()) {
        params.difficulty = models::Problem::stringToDifficulty(request.queryParams.at("difficulty"));
    }
    
    // 执行搜索
    models::ProblemPageResult result = models::g_problemRepository->searchProblems(
        params.keyword,
        params.tags,
        params.difficulty,
        params.page,
        params.pageSize
    );
    
    // 构造响应
    common::JsonValue responseData;
    responseData["page"] = std::to_string(result.page);
    responseData["page_size"] = std::to_string(result.pageSize);
    responseData["total_count"] = std::to_string(result.totalCount);
    responseData["total_pages"] = std::to_string(result.totalPages);
    
    // 构造题目列表JSON
    std::stringstream problemsArray;
    problemsArray << "[";
    
    bool first = true;
    for (const auto& problem : result.problems) {
        if (!first) {
            problemsArray << ",";
        }
        first = false;
        
        problemsArray << "{";
        problemsArray << '"' << "id" << '"' << ":" << '"' << problem->getId() << '"' << ",";
        problemsArray << '"' << "title" << '"' << ":" << '"' << problem->getTitle() << '"' << ",";
        problemsArray << '"' << "difficulty" << '"' << ":" << '"' << 
            models::Problem::difficultyToString(problem->getDifficulty()) << '"' << ",";
        problemsArray << '"' << "tags" << '"' << ":" << '"' << problem->serializeTags() << '"';
        problemsArray << '}';
    }
    
    problemsArray << "]";
    responseData["problems"] = problemsArray.str();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    // 将tags vector转换为字符串
    std::string tagsStr;
    for (size_t i = 0; i < params.tags.size(); i++) {
        tagsStr += params.tags[i];
        if (i < params.tags.size() - 1) tagsStr += ", ";
    }
    
    common::g_logger.info("Problem search executed: keyword='%s', tags='%s', difficulty='%s', duration=%lldms", 
                     params.keyword.c_str(), tagsStr.c_str(), 
                     models::Problem::difficultyToString(params.difficulty).c_str(), duration);
    
    return createSuccessResponse(responseData);
}

long long ProblemController::authenticateRequest(const HttpRequest& request) {
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

long long ProblemController::extractProblemId(const HttpRequest& request) {
    std::string path = request.path;
    size_t lastSlash = path.rfind('/');
    
    if (lastSlash != std::string::npos && lastSlash + 1 < path.size()) {
        try {
            std::string idStr = path.substr(lastSlash + 1);
            return std::stoll(idStr);
        } catch (...) {
            throw common::AppException(common::ErrorCode::INVALID_PARAM, "Invalid problem ID");
        }
    }
    
    throw common::AppException(common::ErrorCode::INVALID_PARAM, "Problem ID not found in path");
}

} // namespace controllers
