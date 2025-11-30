#pragma once

#include "model/shortlink.h"
#include <optional>
#include <vector>

namespace storage {

class ShortLinkDAO {
public:
    // 创建短链接
    bool createShortLink(const model::ShortLink& link);
    
    // 根据ID查找短链接
    std::optional<model::ShortLink> findShortLinkById(uint64_t id);
    
    // 根据短码查找短链接
    std::optional<model::ShortLink> findShortLinkByCode(const std::string& shortCode);
    
    // 根据自定义别名查找短链接
    std::optional<model::ShortLink> findShortLinkByAlias(const std::string& alias);
    
    // 更新短链接
    bool updateShortLink(const model::ShortLink& link);
    
    // 增加访问次数
    bool incrementVisitCount(uint64_t id);
    
    // 添加访问日志
    bool addVisitLog(const model::VisitLog& log);
    
    // 获取短链接的访问日志
    std::vector<model::VisitLog> getVisitLogs(uint64_t linkId, size_t limit = 10);
    
    // 禁用短链接
    bool disableShortLink(uint64_t id);
};

} // namespace storage
