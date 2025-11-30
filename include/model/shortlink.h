#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace model {

struct ShortLink {
    uint64_t id; // 唯一ID
    std::string long_url; // 原始长链接
    std::string short_code; // 短码
    std::string custom_alias; // 自定义别名（如果有）
    uint64_t create_time; // 创建时间戳（秒）
    uint64_t expire_time; // 过期时间戳（秒），0表示永不过期
    bool is_enabled; // 是否启用
    uint64_t visit_count; // 访问次数
};

struct VisitLog {
    uint64_t id; // 唯一ID
    uint64_t link_id; // 关联的短链接ID
    std::string ip; // 访问者IP
    std::string user_agent; // 访问者User-Agent
    uint64_t visit_time; // 访问时间戳（秒）
};

struct ShortLinkStats {
    ShortLink link; // 短链接基本信息
    std::vector<VisitLog> recent_visits; // 最近的访问日志
};

} // namespace model
