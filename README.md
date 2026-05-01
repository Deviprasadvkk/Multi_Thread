# Thread-Safe LRU Cache in C++11

A high-performance, thread-safe Least Recently Used (LRU) cache implementation in modern C++. Built with O(1) get, put, and eviction operations using a doubly-linked list and hash map combination.

## Overview

An LRU cache maintains a fixed number of key-value pairs. When the cache is full and a new item is inserted, the least recently used item is automatically evicted. This is useful for:

- CPU caches
- Database query result caching
- Web service response caching
- Memory-constrained environments

## Features

- **O(1) Operations**: All cache operations (get, put, evict) run in constant time on average
- **Thread-Safe**: Uses a spinlock (atomic-based) for synchronization across multiple threads
- **RAII Pattern**: Automatic lock management with `LockGuard` prevents deadlocks
- **Statistics**: Built-in hit/miss counters for performance analysis
- **No External Dependencies**: Pure C++11, uses only standard library

## Architecture

### Data Structures

- **Hash Map** (`std::unordered_map`): Maps keys to list nodes for O(1) lookup
- **Doubly-Linked List** (`std::list`): Maintains items in recency order
  - Front = most recently used
  - Back = least recently used
- **SpinLock**: Atomic-based lock for thread synchronization

### Why Two Structures?

- A hash map alone doesn't track recency order efficiently
- A list alone requires O(n) lookup time
- Combined, they provide O(1) for all operations

## Building

### Prerequisites

- C++11 compatible compiler (GCC, Clang, MSVC)
- Standard C++ library

### Compilation

```bash
g++ -std=c++11 -Wall -Wextra -pedantic lru_cache.cpp -o lru_cache
```

Or with clang:
```bash
clang++ -std=c++11 -Wall -Wextra lru_cache.cpp -o lru_cache
```

## Usage

### Basic Example

```cpp
#include "lru_cache.cpp"

int main() {
    // Create a cache with capacity 100
    LRUCache<std::string, int> cache(100);
    
    // Insert items
    cache.put("key1", 42);
    cache.put("key2", 99);
    
    // Retrieve items
    int value;
    if (cache.get("key1", value)) {
        std::cout << "Found: " << value << std::endl;  // Prints: Found: 42
    }
    
    // Check membership
    if (cache.contains("key1")) {
        std::cout << "key1 is in cache" << std::endl;
    }
    
    // View statistics
    std::cout << "Hits: " << cache.hits() << std::endl;
    std::cout << "Misses: " << cache.misses() << std::endl;
    std::cout << "Size: " << cache.size() << " / " << cache.capacity() << std::endl;
    
    return 0;
}
```

### API Reference

#### Constructor
```cpp
LRUCache<Key, Value> cache(std::size_t capacity);
```
- Creates a cache with fixed capacity
- Throws `std::invalid_argument` if capacity is 0

#### Methods

| Method | Signature | Time | Purpose |
|--------|-----------|------|---------|
| `get` | `bool get(const Key& key, Value& outValue)` | O(1) | Retrieve value by key, update recency |
| `put` | `void put(const Key& key, const Value& value)` | O(1) | Insert or update key-value pair |
| `contains` | `bool contains(const Key& key) const` | O(1) | Check if key exists |
| `size` | `std::size_t size() const` | O(1) | Current number of items |
| `capacity` | `std::size_t capacity() const` | O(1) | Maximum capacity |
| `hits` | `std::size_t hits() const` | O(1) | Successful get() calls |
| `misses` | `std::size_t misses() const` | O(1) | Failed get() calls |
| `clear` | `void clear()` | O(n) | Empty cache and reset stats |

## Thread Safety

The cache is fully thread-safe:

- All operations are protected by a **spinlock** (`SpinLock` class)
- Uses atomic operations with proper memory ordering (`memory_order_acquire`/`memory_order_release`)
- `LockGuard` implements RAII to ensure locks are released even if exceptions occur
- Safe for concurrent reads and writes from multiple threads

## Performance

### Complexity Analysis

| Operation | Time | Space |
|-----------|------|-------|
| `get()` | O(1) avg | O(1) |
| `put()` | O(1) avg | O(1) |
| `evict` | O(1) | O(1) |
| `contains()` | O(1) avg | O(1) |

The "avg" qualifications are due to hash table collision possibilities, but in practice, these are negligible for well-distributed keys.

### When to Use

**Good for:**
- High hit-rate workloads (Zipfian access patterns)
- Memory-constrained environments
- Real-time systems needing predictable latency
- Multi-threaded applications

**Not ideal for:**
- Extremely high contention (millions of ops/sec on single core)
- Very large capacities (consider segmented/sharded approaches)

## Example Output

```
After inserting A, B, C:
Cache state [MRU -> LRU]: (C: 3) (B: 2) (A: 1)

Got A = 1
A has been moved to MRU position.

After inserting D (cache is full, so B is evicted):
Cache state [MRU -> LRU]: (D: 4) (A: 1) (C: 3)

Checking membership:
Contains B? no
Contains A? yes

Cache statistics:
Hits: 1
Misses: 0
```

## Implementation Details

### Why SpinLock Instead of std::mutex?

The custom spinlock is used for teaching purposes and works well in C++11 without requiring threading headers. For production code, consider:

- **std::mutex**: Better for high contention or long critical sections
- **Reader-writer lock**: If reads vastly outnumber writes
- **Sharded locks**: For ultra-high throughput on multi-core systems

### Memory Ordering

- `memory_order_acquire` on lock: Prevents subsequent operations from reordering before the lock
- `memory_order_release` on unlock: Prevents prior operations from reordering after the lock
- Together: Guarantees threads see consistent cache state

## Limitations & Future Improvements

1. **Single Mutex**: All operations serialize; consider sharded locks for higher concurrency
2. **Open-Addressing Hash Map**: Could replace `std::unordered_map` for better locality
3. **Approximate LRU**: For very high concurrency, approximate LRU is faster
4. **Move Semantics**: Could optimize for C++17 with better move support
5. **Custom Allocators**: Could reduce allocation overhead

## Testing

Run the included demo:
```bash
./lru_cache
```

For more extensive testing, add unit tests using a framework like Google Test.

## License

This project is open source and available for educational and commercial use.

## Author

Deviprasad - Resume Projects Series

---

**For questions, optimizations, or contributions, please feel free to open an issue or pull request.**
