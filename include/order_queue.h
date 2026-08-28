#pragma once
#include "types.h"

class OrderQueue {
private:
    Order* head{nullptr};
    Order* tail{nullptr};
    uint32_t total_volume{0}; // Track total shares at this price level

public:
    OrderQueue() = default;

    bool is_empty() const {
        return head == nullptr;
    }

    Order* get_head() const {
        return head;
    }

    uint32_t get_volume() const {
        return total_volume;
    }

    // O(1) Insertion at the back (Price-Time Priority)
    void push_back(Order* order) {
        if (!order) return;
        
        order->prev = tail;
        order->next = nullptr;

        if (tail) {
            tail->next = order;
        } else {
            head = order; // Queue was empty
        }
        tail = order;
        
        total_volume += order->quantity;
    }

    // O(1) Deletion from anywhere in the queue
    void erase(Order* order) {
        if (!order) return;

        // Bypass the order in the forward direction
        if (order->prev) {
            order->prev->next = order->next;
        } else {
            head = order->next; // Order was at the front
        }

        // Bypass the order in the backward direction
        if (order->next) {
            order->next->prev = order->prev;
        } else {
            tail = order->prev; // Order was at the back
        }

        // Clean up the removed order's pointers
        order->prev = nullptr;
        order->next = nullptr;
        
        total_volume -= order->quantity;
    }
};