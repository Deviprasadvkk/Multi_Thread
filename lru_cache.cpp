#include <atomic>
#include <iostream>
#include <list>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

class SpinLock
{
public:
    void lock()
    {
        while (flag_.test_and_set(std::memory_order_acquire))
        {
        }
    }

    void unlock()
    {
        flag_.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag_;

public:
    SpinLock() : flag_(ATOMIC_FLAG_INIT) {}
};

class LockGuard
{
public:
    explicit LockGuard(SpinLock &lock) : lock_(lock)
    {
        lock_.lock();
    }

    ~LockGuard()
    {
        lock_.unlock();
    }

    LockGuard(const LockGuard &) = delete;
    LockGuard &operator=(const LockGuard &) = delete;

private:
    SpinLock &lock_;
};

template <typename Key, typename Value>
class LRUCache
{
public:
    explicit LRUCache(std::size_t capacity) : capacity_(capacity)
    {
        if (capacity_ == 0)
        {
            throw std::invalid_argument("LRUCache capacity must be greater than zero");
        }
    }

    bool get(const Key &key, Value &outValue)
    {
        LockGuard lock(lock_);

        typename std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator>::iterator mapIt =
            index_.find(key);
        if (mapIt == index_.end())
        {
            ++misses_;
            return false;
        }

        items_.splice(items_.begin(), items_, mapIt->second);
        ++hits_;
        outValue = mapIt->second->second;
        return true;
    }

    void put(const Key &key, const Value &value)
    {
        LockGuard lock(lock_);

        typename std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator>::iterator mapIt =
            index_.find(key);
        if (mapIt != index_.end())
        {
            mapIt->second->second = value;
            items_.splice(items_.begin(), items_, mapIt->second);
            return;
        }

        if (items_.size() == capacity_)
        {
            std::pair<Key, Value> &lruEntry = items_.back();
            index_.erase(lruEntry.first);
            items_.pop_back();
        }

        items_.push_front(std::make_pair(key, value));
        index_[items_.front().first] = items_.begin();
    }

    bool contains(const Key &key) const
    {
        LockGuard lock(lock_);
        return index_.find(key) != index_.end();
    }

    std::size_t size() const
    {
        LockGuard lock(lock_);
        return items_.size();
    }

    std::size_t capacity() const
    {
        return capacity_;
    }

    std::size_t hits() const
    {
        LockGuard lock(lock_);
        return hits_;
    }

    std::size_t misses() const
    {
        LockGuard lock(lock_);
        return misses_;
    }

    void clear()
    {
        LockGuard lock(lock_);
        items_.clear();
        index_.clear();
        hits_ = 0;
        misses_ = 0;
    }

    void print_state() const
    {
        LockGuard lock(lock_);
        std::cout << "Cache state [MRU -> LRU]: ";
        for (typename std::list<std::pair<Key, Value>>::const_iterator it = items_.begin();
             it != items_.end();
             ++it)
        {
            std::cout << "(" << it->first << ": " << it->second << ") ";
        }
        std::cout << '\n';
    }

private:
    typedef std::list<std::pair<Key, Value>> ListType;
    typedef typename ListType::iterator ListIterator;

    ListType items_;
    std::unordered_map<Key, ListIterator> index_;
    mutable SpinLock lock_;
    std::size_t capacity_;
    std::size_t hits_ = 0;
    std::size_t misses_ = 0;

public:
    LRUCache(const LRUCache &) = delete;
    LRUCache &operator=(const LRUCache &) = delete;
};

int main()
{
    LRUCache<std::string, int> cache(3);

    cache.put("A", 1);
    cache.put("B", 2);
    cache.put("C", 3);
    cache.print_state();

    int value = 0;
    if (cache.get("A", value))
    {
        std::cout << "A = " << value << '\n';
    }

    cache.put("D", 4);
    cache.print_state();

    std::cout << "Contains B? " << (cache.contains("B") ? "yes" : "no") << '\n';
    std::cout << "Contains A? " << (cache.contains("A") ? "yes" : "no") << '\n';
    std::cout << "Hits: " << cache.hits() << '\n';
    std::cout << "Misses: " << cache.misses() << '\n';

    return 0;
}
