#include "service/shortlink_service.h"
#include "utils/logger.h"
#include "utils/time.h"
#include <regex>

namespace service {

ShortLinkService::ShortLinkService() : cache_(1000) {
}

ShortLinkService::ShortLinkService(size_t cacheSize) : cache_(cacheSize) {
}

ShortLinkService::CreateShortLinkResponse ShortLinkService::createShortLink(
    const CreateShortLinkRequest& request) {
    
    CreateShortLinkResponse response;
    
    // 验证长链接
    if (!validateLongUrl(request.long_url)) {
        response.success = false;
        response.error_message = "Invalid long URL format";
        return response;
    }
    
    // 检查是否存在相同的长链接
    // auto existingLink = dao_.findShortLinkByLongUrl(request.long_url);
    // if (existingLink) {
    //     response.success = true;
    //     response.link = *existingLink;
    //     return response;
    // }
    
    // 创建短链接
    model::ShortLink link;
    link.long_url = request.long_url;
    link.short_code = generateShortCode();
    link.custom_alias = request.custom_alias;
    link.create_time = utils::TimeUtils::getCurrentTimestamp();
    // 计算过期时间
    uint64_t expireTime = 0;
    if (request.expire_seconds > 0) {
        expireTime = utils::TimeUtils::getCurrentTimestamp() + request.expire_seconds;
    }
    link.expire_time = expireTime;
    link.is_enabled = true;
    link.visit_count = 0;
    
    if (dao_.createShortLink(link)) {
        response.success = true;
        response.link = link;
        
        // 添加到缓存
        if (!link.short_code.empty()) {
            cache_.put(link.short_code, link);
        }
        if (!link.custom_alias.empty()) {
            cache_.put(link.custom_alias, link);
        }
    } else {
        response.success = false;
        response.error_message = "Failed to create short link";
    }
    
    return response;
}

ShortLinkService::ResolveShortLinkResponse ShortLinkService::resolveShortLink(const ResolveShortLinkRequest& request) {
    
    ResolveShortLinkResponse response;
    response.success = false;
    
    // 首先从缓存中查找
    auto cachedLink = cache_.get(request.short_code);
    
    model::ShortLink link;
    bool found = false;
    
    if (cachedLink) {
        link = *cachedLink;
        found = true;
        LOG_INFO("Short link resolved from cache: ", request.short_code);
    } else {
        // 从数据库中查找
        auto dbLink = dao_.findShortLinkByCode(request.short_code);
        if (dbLink) {
            link = *dbLink;
            found = true;
            
            // 添加到缓存
            cache_.put(request.short_code, link);
            
            LOG_INFO("Short link resolved from database: ", request.short_code);
        }
    }
    
    if (!found) {
        response.error_message = "Short link not found";
        LOG_ERROR("Failed to resolve short link: not found - ", request.short_code);
        return response;
    }
    
    // 检查是否已禁用
    if (!link.is_enabled) {
        response.error_message = "Short link is disabled";
        LOG_ERROR("Failed to resolve short link: disabled - ", request.short_code);
        return response;
    }
    
    // 检查是否已过期
    if (utils::TimeUtils::isExpired(link.expire_time)) {
        response.error_message = "Short link has expired";
        LOG_ERROR("Failed to resolve short link: expired - ", request.short_code);
        return response;
    }
    
    // 增加访问次数
    if (!dao_.incrementVisitCount(link.id)) {
        LOG_ERROR("Failed to increment visit count for short link: ", link.id);
        // 这里不返回错误，因为访问次数统计失败不应该影响用户访问
    }
    
    // 添加访问日志
    model::VisitLog log;
    log.link_id = link.id;
    log.ip = request.ip;
    log.user_agent = request.user_agent;
    log.visit_time = utils::TimeUtils::getCurrentTimestamp();
    
    if (!dao_.addVisitLog(log)) {
        LOG_ERROR("Failed to add visit log for short link: ", link.id);
        // 这里也不返回错误，因为日志记录失败不应该影响用户访问
    }
    
    // 更新缓存中的访问次数
    link.visit_count++;
    cache_.put(request.short_code, link);
    
    response.success = true;
    response.long_url = link.long_url;
    
    return response;
}

ShortLinkService::GetShortLinkStatsResponse ShortLinkService::getShortLinkStats(
    const GetShortLinkStatsRequest& request) {
    
    GetShortLinkStatsResponse response;
    
    auto link = dao_.findShortLinkById(request.link_id);
    if (link) {
        response.success = true;
        response.stats.link = *link;
        response.stats.recent_visits = dao_.getVisitLogs(request.link_id, request.recent_visits_limit);
    } else {
        response.success = false;
        response.error_message = "Short link not found";
    }
    
    return response;
}

ShortLinkService::DisableShortLinkResponse ShortLinkService::disableShortLink(
    const DisableShortLinkRequest& request) {
    
    DisableShortLinkResponse response;
    
    auto link = dao_.findShortLinkById(request.link_id);
    if (link) {
        if (!link->is_enabled) {
            response.success = false;
            response.error_message = "Short link is already disabled";
        } else {
            link->is_enabled = false;
            if (dao_.updateShortLink(*link)) {
                response.success = true;
                
                // 从缓存中移除
                if (!link->short_code.empty()) {
                    cache_.remove(link->short_code);
                }
                if (!link->custom_alias.empty()) {
                    cache_.remove(link->custom_alias);
                }
            } else {
                response.success = false;
                response.error_message = "Failed to disable short link";
            }
        }
    } else {
        response.success = false;
        response.error_message = "Short link not found";
    }
    
    return response;
}

bool ShortLinkService::validateLongUrl(const std::string& url) {
    if (url.empty()) {
        return false;
    }
    
    // 正则表达式：匹配http://或https://开头的URL
    const std::regex urlRegex(R"(^https?://[^\s/$.?#].[^\s]*$)");
    
    return std::regex_match(url, urlRegex);
}

std::string ShortLinkService::generateShortCode() {
    // 生成随机短码的逻辑
    // 这里使用简单的实现，实际项目中可以使用更复杂的算法
    static const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string code;
    
    // 生成6位随机字符
    for (int i = 0; i < 6; ++i) {
        code += chars[rand() % chars.size()];
    }
    
    return code;
}

// void ShortLinkService::updateVisitCount(const std::string& shortCodeOrAlias, const std::string& ip, const std::string& userAgent) {
//     // 异步更新访问计数的逻辑
//     // 这里使用简单的实现，实际项目中可以使用线程池或消息队列
//     
//     // 查找短链接
//     auto link = dao_.findShortLinkByCodeOrAlias(shortCodeOrAlias);
//     if (link) {
//         // 更新访问计数
//         link->visit_count++;
//         dao_.updateShortLink(*link);
//         
//         // 记录访问日志
//         model::VisitLog log;
//         log.link_id = link->id;
//         log.ip = ip;
//         log.user_agent = userAgent;
//         log.visit_time = utils::TimeUtils::getCurrentTimestamp();
//         dao_.addVisitLog(log);
//     }
// }

} // namespace service