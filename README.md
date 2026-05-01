# Thread-Safe LRU Cache in C++11

A fast, thread-safe Least Recently Used (LRU) cache with O(1) operations. Uses a hash map + doubly-linked list for optimal performance.

## Quick Start

### Build
```bash
g++ -std=c++11 -Wall -Wextra lru_cache.cpp -o lru_cache
./lru_cache
```

### Usage
```cpp
LRUCache<std::string, int> cache(100);

cache.put("key1", 42);
int value;
if (cache.get("key1", value)) {
    std::cout << "Found: " << value << std::endl;
}

std::cout << "Hits: " << cache.hits() << std::endl;
std::cout << "Size: " << cache.size() << " / " << cache.capacity() << std::endl;
```

## API

| Method | Returns | Time |
|--------|---------|------|
| `put(key, value)` | void | O(1) |
| `get(key, outValue)` | bool | O(1) |
| `contains(key)` | bool | O(1) |
| `size()` | size_t | O(1) |
| `capacity()` | size_t | O(1) |
| `hits()` | size_t | O(1) |
| `misses()` | size_t | O(1) |
| `clear()` | void | O(n) |

## Features

- **O(1) get/put/evict** using hash map + doubly-linked list
- **Thread-safe** with atomic spinlock
- **RAII lock guard** prevents deadlocks
- **Statistics** for cache analysis (hits/misses)
- **No dependencies** - pure C++11 standard library

## How It Works

The cache maintains two data structures in sync:

1. **Hash Map**: Maps keys → list nodes (O(1) lookup)
2. **Doubly-Linked List**: Tracks recency order (O(1) reordering)

When you `get()` an item, it moves to the front (most recently used).
When the cache is full, the back item (least recently used) is evicted.

## Thread Safety

- All operations protected by atomic spinlock
- Proper memory ordering (`acquire`/`release`)
- Safe for concurrent multi-threaded access
- RAII ensures locks released even on exceptions

## Example Output

```
After inserting A, B, C:
Cache state [MRU -> LRU]: (C: 3) (B: 2) (A: 1)

Got A = 1
After inserting D (cache full, B evicted):
Cache state [MRU -> LRU]: (D: 4) (A: 1) (C: 3)

Hits: 1, Misses: 0
```

