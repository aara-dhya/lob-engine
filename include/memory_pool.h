#pragma once
#include <vector>
#include <stdexcept>
#include <cstddef>

template <typename T, size_t PoolSize>
class MemoryPool {
private:
    T pool[PoolSize]; // The massive pre-allocated block of memory
    std::vector<size_t> free_indices; // Stack of available slots

public:
    MemoryPool() {
        // Pre-allocate the free list at startup to avoid runtime allocations
        free_indices.reserve(PoolSize);
        // Push all indices onto the free list (backwards so we use index 0 first)
        for (size_t i = PoolSize; i > 0; --i) {
            free_indices.push_back(i - 1);
        }
    }

    // O(1) Allocation
    T* allocate() {
        if (free_indices.empty()) {
            throw std::bad_alloc(); // The pool is completely full
        }
        size_t index = free_indices.back();
        free_indices.pop_back();
        return &pool[index];
    }

    // O(1) Deallocation
    void deallocate(T* ptr) {
        // Pointer arithmetic to figure out exactly where this object lives in the array
        size_t index = ptr - pool; 
        free_indices.push_back(index);
    }
};