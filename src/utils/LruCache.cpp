#include "LruCache.h"
#include "Logger.h"

namespace utils {

LruCache g_cache;

void LruCache::init(int capacity, int ttl_seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    capacity_ = capacity;
    ttl_seconds_ = ttl_seconds;
    LOG_INFO("LruCache initialized with capacity " + std::to_string(capacity_) + " and TTL " + std::to_string(ttl_seconds_) + " seconds");
}

nlohmann::json LruCache::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = cache_map_.find(key);
    if (it == cache_map_.end()) {
        return nlohmann::json();
    }

    // 检查是否过期
    if (isExpired(*it->second)) {
        cache_list_.erase(it->second);
        cache_map_.erase(it);
        LOG_DEBUG("Cache item expired and removed: " + key);
        return nlohmann::json();
    }

    // 移动到列表前端（表示最近使用）
    cache_list_.splice(cache_list_.begin(), cache_list_, it->second);

    return it->second->value;
}

void LruCache::set(const std::string& key, const nlohmann::json& value) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 清理过期项
    cleanupExpiredItems();

    auto it = cache_map_.find(key);
    if (it != cache_map_.end()) {
        // 存在则更新值和过期时间，并移动到列表前端
        it->second->value = value;
        it->second->expire_time = std::chrono::system_clock::now() + std::chrono::seconds(ttl_seconds_);
        cache_list_.splice(cache_list_.begin(), cache_list_, it->second);
    } else {
        // 不存在则添加新项
        if (cache_list_.size() >= static_cast<size_t>(capacity_)) {
            // 容量已满，删除最后一个项（最少使用）
            auto last = cache_list_.back();
            cache_map_.erase(last.key);
            cache_list_.pop_back();
            LOG_DEBUG("Cache capacity reached, removed least recently used item: " + last.key);
        }

        // 添加新项到列表前端
        auto expire_time = std::chrono::system_clock::now() + std::chrono::seconds(ttl_seconds_);
        cache_list_.emplace_front(key, value, expire_time);
        cache_map_[key] = cache_list_.begin();
    }
}

void LruCache::remove(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = cache_map_.find(key);
    if (it != cache_map_.end()) {
        cache_list_.erase(it->second);
        cache_map_.erase(it);
        LOG_DEBUG("Cache item removed: " + key);
    }
}

void LruCache::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_list_.clear();
    cache_map_.clear();
    LOG_DEBUG("Cache cleared");
}

int LruCache::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cache_list_.size();
}

bool LruCache::isExpired(const CacheItem& item) const {
    return std::chrono::system_clock::now() > item.expire_time;
}

void LruCache::cleanupExpiredItems() {
    auto now = std::chrono::system_clock::now();
    while (!cache_list_.empty() && now > cache_list_.back().expire_time) {
        auto last = cache_list_.back();
        cache_map_.erase(last.key);
        cache_list_.pop_back();
        LOG_DEBUG("Expired cache item removed: " + last.key);
    }
}

} // namespace utils
