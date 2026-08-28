#pragma once
#include <cstdint>

enum class Side {
    BUY,
    SELL
};

struct Order {
    uint64_t order_id;
    uint64_t price;     // Prices are usually stored as integers (e.g., $150.25 -> 15025) to avoid floating-point inaccuracies
    uint32_t quantity;
    Side side;
    
    // We will add pointers for the intrusive doubly linked list here in the next phase
};