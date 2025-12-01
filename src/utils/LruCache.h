#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <unordered_map>
#include <list>
#include <mutex>
#include <chrono>
#include <string>
#include <nlohmann/json.hpp>

namespace utils {

struct CacheItem {
    std::string key;
    nlohmann::json value;
    std::chrono::system_clock::time_point expire_time;

    CacheItem(std::string k, nlohmann::json v, std::chrono::system_clock::time_point et)
        : key(std::move(k)), value(std::move(v)), expire_time(et) {}
};

class LruCache {
public:
    LruCache() = default;
    ~LruCache() = default;

    // 初始化缓存
    void init(int capacity, int ttl_seconds);

    // 获取缓存项
    nlohmann::json get(const std::string& key);

    // 设置缓存项
    void set(const std::string& key, const nlohmann::json& value);

    // 删除缓存项
    void remove(const std::string& key);

    // 清空缓存
    void clear();

    // 获取缓存大小
    int size() const;

private:
    // 检查缓存项是否过期
    bool isExpired(const CacheItem& item) const;

    // 清理过期的缓存项
    void cleanupExpiredItems();

private:
    int capacity_ = 100;
    int ttl_seconds_ = 300;
    std::list<CacheItem> cache_list_;
    std::unordered_map<std::string, std::list<CacheItem>::iterator> cache_map_;
    mutable std::mutex mutex_;
};

// 全局缓存实例
extern LruCache g_cache;

} // namespace utils

#endif // LRU_CACHE_H
