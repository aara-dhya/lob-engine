#pragma once
#include <map>
#include <unordered_map>
#include "types.h"
#include "memory_pool.h"
#include "order_queue.h"

class LimitOrderBook {
private:
    // 1. The Memory Pool: Pre-allocating 1,000,000 orders to avoid runtime OS calls
    MemoryPool<Order, 1000000> order_pool;

    // 2. The Price Levels (Red-Black Trees) - O(log n) insertion/deletion
    // Bids: We want the HIGHEST price at the top, so we use std::greater
    std::map<uint64_t, OrderQueue, std::greater<uint64_t>> bids;
    
    // Asks: We want the LOWEST price at the top, so we use std::less
    std::map<uint64_t, OrderQueue, std::less<uint64_t>> asks;

    // 3. The Order Lookup (Hash Map) - O(1) lookups
    std::unordered_map<uint64_t, Order*> order_map;

public:
    LimitOrderBook() {
        // Pre-reserve hash map buckets at startup so it never has to rehash during live trading
        order_map.reserve(1000000);
    }

    // Basic Add Order Logic (Resting in the book)
    void add_order(uint64_t order_id, uint64_t price, uint32_t quantity, Side side) {
        // Grab a pre-allocated order from the pool in O(1) time
        Order* new_order = order_pool.allocate();
        new_order->order_id = order_id;
        new_order->price = price;
        new_order->quantity = quantity;
        new_order->side = side;

        // Add to the correct Red-Black tree and Intrusive Queue
        if (side == Side::BUY) {
            bids[price].push_back(new_order);
        } else {
            asks[price].push_back(new_order);
        }

        // Store in the hash map for instant future lookups
        order_map[order_id] = new_order;
    }

    // O(1) Cancel Order Logic
    void cancel_order(uint64_t order_id) {
        auto it = order_map.find(order_id);
        if (it == order_map.end()) {
            return; // Order not found
        }

        Order* order_to_cancel = it->second;
        
        // Remove from the intrusive linked list queue in O(1) time
        if (order_to_cancel->side == Side::BUY) {
            bids[order_to_cancel->price].erase(order_to_cancel);
            
            // Clean up the price level if the queue is now empty
            if (bids[order_to_cancel->price].is_empty()) {
                bids.erase(order_to_cancel->price);
            }
        } else {
            asks[order_to_cancel->price].erase(order_to_cancel);
            
            if (asks[order_to_cancel->price].is_empty()) {
                asks.erase(order_to_cancel->price);
            }
        }

        // Remove from hash map and return the memory to the pool
        order_map.erase(it);
        order_pool.deallocate(order_to_cancel);
    }
};