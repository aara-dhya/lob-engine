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

    // O(log n) Matching and Routing Logic
    void add_order(uint64_t order_id, uint64_t price, uint32_t quantity, Side side) {
        
        // 1. MATCHING ENGINE LOGIC (Crossing the spread)
        if (side == Side::BUY) {
            // While we have quantity to buy, AND there are asks available, 
            // AND the best ask price is less than or equal to our buy price
            while (quantity > 0 && !asks.empty() && asks.begin()->first <= price) {
                auto best_ask_iter = asks.begin();
                OrderQueue& best_ask_queue = best_ask_iter->second;
                Order* resting_ask = best_ask_queue.get_head();

                // Trade at the resting order's quantity or our remaining quantity, whichever is smaller
                uint32_t trade_qty = std::min(quantity, resting_ask->quantity);
                
                // Execute Trade (In a real system, a TCP execution message is sent to clients here)
                quantity -= trade_qty;
                resting_ask->quantity -= trade_qty;

                // If the resting order is fully filled, remove it from the queue and memory pool
                if (resting_ask->quantity == 0) {
                    best_ask_queue.erase(resting_ask);
                    order_map.erase(resting_ask->order_id);
                    order_pool.deallocate(resting_ask);
                }

                // If the price level queue is empty, remove the price level entirely
                if (best_ask_queue.is_empty()) {
                    asks.erase(best_ask_iter);
                }
            }
        } else {
            // SELL LOGIC: Match against Bids
            // While we have quantity to sell, AND there are bids available,
            // AND the best bid price is greater than or equal to our sell price
            while (quantity > 0 && !bids.empty() && bids.begin()->first >= price) {
                auto best_bid_iter = bids.begin();
                OrderQueue& best_bid_queue = best_bid_iter->second;
                Order* resting_bid = best_bid_queue.get_head();

                uint32_t trade_qty = std::min(quantity, resting_bid->quantity);
                
                quantity -= trade_qty;
                resting_bid->quantity -= trade_qty;

                if (resting_bid->quantity == 0) {
                    best_bid_queue.erase(resting_bid);
                    order_map.erase(resting_bid->order_id);
                    order_pool.deallocate(resting_bid);
                }

                if (best_bid_queue.is_empty()) {
                    bids.erase(best_bid_iter);
                }
            }
        }

        // 2. ADD REMAINING LIQUIDITY TO THE BOOK
        // If the incoming order wasn't fully filled by the matching engine, rest the remainder
        if (quantity > 0) {
            Order* new_order = order_pool.allocate();
            new_order->order_id = order_id;
            new_order->price = price;
            new_order->quantity = quantity;
            new_order->side = side;

            if (side == Side::BUY) {
                bids[price].push_back(new_order);
            } else {
                asks[price].push_back(new_order);
            }
            order_map[order_id] = new_order;
        }
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