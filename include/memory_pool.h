#pragma once
#include <vector>
#include <stdexcept>
#include <cstddef>

template <typename T, size_t PoolSize>
class MemoryPool {
private:
    std::vector<T> pool;              // Moves the massive 40MB block to the Heap
    std::vector<size_t> free_indices;

public:
    // Initialize the pool vector to exactly PoolSize at startup
    MemoryPool() : pool(PoolSize) {
        free_indices.reserve(PoolSize);
        for (size_t i = PoolSize; i > 0; --i) {
            free_indices.push_back(i - 1);
        }
    }

    T* allocate() {
        if (free_indices.empty()) {
            throw std::bad_alloc(); 
        }
        size_t index = free_indices.back();
        free_indices.pop_back();
        return &pool[index];
    }

    void deallocate(T* ptr) {
        // .data() gets the underlying raw heap pointer from the vector
        size_t index = ptr - pool.data(); 
        free_indices.push_back(index);
    }
};