#pragma once

#include <unordered_map>
#include <list>
#include <mutex>
#include <optional>

namespace utils {

template<typename K, typename V>
class LRUCache {
public:
    explicit LRUCache(size_t capacity)
        : capacity_(capacity) {
    }
    
    void put(const K& key, const V& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = map_.find(key);
        if (it != map_.end()) {
            // 键已存在，移动到列表头部
            list_.erase(it->second);
        } else if (list_.size() >= capacity_) {
            // 缓存已满，移除最后一个元素
            K lastKey = list_.back();
            list_.pop_back();
            map_.erase(lastKey);
        }
        
        // 添加到列表头部
        list_.push_front(key);
        map_[key] = list_.begin();
        
        // 更新值
        valueMap_[key] = value;
    }
    
    std::optional<V> get(const K& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = map_.find(key);
        if (it == map_.end()) {
            return std::nullopt;
        }
        
        // 移动到列表头部
        list_.erase(it->second);
        list_.push_front(key);
        map_[key] = list_.begin();
        
        return valueMap_[key];
    }
    
    void remove(const K& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = map_.find(key);
        if (it != map_.end()) {
            list_.erase(it->second);
            map_.erase(it);
            valueMap_.erase(key);
        }
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return list_.size();
    }
    
    bool contains(const K& key) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return map_.contains(key);
    }
    
private:
    size_t capacity_;
    std::list<K> list_; // 按访问顺序排列，头部是最近访问的
    std::unordered_map<K, typename std::list<K>::iterator> map_; // 键到列表迭代器的映射
    std::unordered_map<K, V> valueMap_; // 键到值的映射
    mutable std::mutex mutex_; // 用于线程安全
};

} // namespace utils
