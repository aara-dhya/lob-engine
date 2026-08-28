#pragma once
#include <cstdint>

enum class Side {
    BUY,
    SELL
};

struct Order {
    uint64_t order_id{0};
    uint64_t price{0};
    uint32_t quantity{0};
    Side side{Side::BUY};
    
    // Intrusive Linked List Hooks
    Order* prev{nullptr};
    Order* next{nullptr};
};