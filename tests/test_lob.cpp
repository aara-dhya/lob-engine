#include <gtest/gtest.h>
#include "../include/limit_order_book.h"

TEST(LimitOrderBookTest, PartialFillLogic) {
    LimitOrderBook lob;

    // 1. Add a resting SELL order (Ask) for 100 shares at $150
    // Price = 15000 (Integer representation of $150.00)
    lob.add_order(1, 15000, 100, Side::SELL);
    
    // Verify the book registered the Ask
    EXPECT_EQ(lob.get_best_ask(), 15000);

    // 2. Add an aggressive BUY order (Bid) for 300 shares at $150
    // This should instantly execute against the 100 shares, and rest the remaining 200.
    lob.add_order(2, 15000, 300, Side::BUY);

    // 3. Verify the state of the book after the trade
    // The resting Sell order should be completely wiped out
    EXPECT_EQ(lob.get_best_ask(), 0);
    
    // The remaining 200 shares of the Buy order should now be resting as the best Bid
    EXPECT_EQ(lob.get_best_bid(), 15000);
}