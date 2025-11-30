#pragma once

#include "model/shortlink.h"
#include "storage/shortlink_dao.h"
#include "utils/lru_cache.h"
#include <optional>
#include <string>

namespace service {

class ShortLinkService {
public:
    ShortLinkService();
    explicit ShortLinkService(size_t cacheSize);
    
    // 创建短链接
    struct CreateShortLinkRequest {
        std::string long_url;
        uint64_t expire_seconds = 0; // 0表示永不过期
        std::string custom_alias;
    };
    
    struct CreateShortLinkResponse {
        bool success;
        std::string error_message;
        model::ShortLink link;
    };
    
    CreateShortLinkResponse createShortLink(const CreateShortLinkRequest& request);
    
    // 解析短链接
    struct ResolveShortLinkRequest {
        std::string short_code;
        std::string ip;
        std::string user_agent;
    };
    
    struct ResolveShortLinkResponse {
        bool success;
        std::string error_message;
        std::string long_url;
    };
    
    ResolveShortLinkResponse resolveShortLink(const ResolveShortLinkRequest& request);
    
    // 获取短链接统计信息
    struct GetShortLinkStatsRequest {
        uint64_t link_id;
        size_t recent_visits_limit = 10;
    };
    
    struct GetShortLinkStatsResponse {
        bool success;
        std::string error_message;
        model::ShortLinkStats stats;
    };
    
    GetShortLinkStatsResponse getShortLinkStats(const GetShortLinkStatsRequest& request);
    
    // 禁用短链接
    struct DisableShortLinkRequest {
        uint64_t link_id;
    };
    
    struct DisableShortLinkResponse {
        bool success;
        std::string error_message;
    };
    
    DisableShortLinkResponse disableShortLink(const DisableShortLinkRequest& request);
    
private:
    // 生成短码
    std::string generateShortCode();
    
    // 验证长链接格式
    bool validateLongUrl(const std::string& url);
    
    // 验证自定义别名格式
    bool validateCustomAlias(const std::string& alias);
    
    storage::ShortLinkDAO dao_;
    utils::LRUCache<std::string, model::ShortLink> cache_;
};

} // namespace service
