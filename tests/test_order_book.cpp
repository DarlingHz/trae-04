#include <gtest/gtest.h>
#include "OrderBook.h"

class OrderBookTest : public ::testing::Test {
pprotected:
    OrderBook order_book_ = OrderBook("BTC/USD");
};

TEST_F(OrderBookTest, SubmitLimitOrder) {
    auto [order, trades] = order_book_.submit_order(
        "user1", Order::Side::BUY, Order::Type::LIMIT, 1000000000, 100000000
    );
    
    EXPECT_EQ(order->user_id, "user1");
    EXPECT_EQ(order->symbol, "BTC/USD");
    EXPECT_EQ(order->side, Order::Side::BUY);
    EXPECT_EQ(order->type, Order::Type::LIMIT);
    EXPECT_EQ(order->price, 1000000000);
    EXPECT_EQ(order->quantity, 100000000);
    EXPECT_EQ(order->filled_quantity, 0);
    EXPECT_FALSE(order->is_cancelled);
    EXPECT_TRUE(trades.empty());
    
    EXPECT_EQ(order_book_.get_bid_count(), 1);
    EXPECT_EQ(order_book_.get_ask_count(), 0);
    EXPECT_EQ(order_book_.get_total_order_count(), 1);
}

TEST_F(OrderBookTest, SubmitMarketOrder) {
    // 先添加一个限价卖单
    order_book_.submit_order(
        "user1", Order::Side::SELL, Order::Type::LIMIT, 1000000000, 100000000
    );
    
    // 提交市价买单
    auto [order, trades] = order_book_.submit_order(
        "user2", Order::Side::BUY, Order::Type::MARKET, 0, 50000000
    );
    
    EXPECT_EQ(order->user_id, "user2");
    EXPECT_EQ(order->type, Order::Type::MARKET);
    EXPECT_EQ(order->filled_quantity, 50000000);
    EXPECT_EQ(order->quantity, 50000000);
    
    EXPECT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].price, 1000000000);
    EXPECT_EQ(trades[0].quantity, 50000000);
    
    EXPECT_EQ(order_book_.get_bid_count(), 0);
    EXPECT_EQ(order_book_.get_ask_count(), 1);
}

TEST_F(OrderBookTest, MatchOrders) {
    // 添加多个卖单
    order_book_.submit_order(
        "user1", Order::Side::SELL, Order::Type::LIMIT, 1000000000, 50000000
    );
    order_book_.submit_order(
        "user2", Order::Side::SELL, Order::Type::LIMIT, 1001000000, 50000000
    );
    order_book_.submit_order(
        "user3", Order::Side::SELL, Order::Type::LIMIT, 1002000000, 50000000
    );
    
    // 提交一个大额买单
    auto [order, trades] = order_book_.submit_order(
        "user4", Order::Side::BUY, Order::Type::LIMIT, 1001500000, 120000000
    );
    
    EXPECT_EQ(order->filled_quantity, 100000000);  // 50M + 50M
    EXPECT_EQ(trades.size(), 2);
    EXPECT_EQ(trades[0].price, 1000000000);
    EXPECT_EQ(trades[0].quantity, 50000000);
    EXPECT_EQ(trades[1].price, 1001000000);
    EXPECT_EQ(trades[1].quantity, 50000000);
    
    EXPECT_EQ(order_book_.get_bid_count(), 1);  // 剩余20M买单
    EXPECT_EQ(order_book_.get_ask_count(), 1);  // 剩余50M卖单
}

TEST_F(OrderBookTest, CancelOrder) {
    auto [order, _] = order_book_.submit_order(
        "user1", Order::Side::BUY, Order::Type::LIMIT, 1000000000, 100000000
    );
    
    EXPECT_EQ(order_book_.get_bid_count(), 1);
    
    bool success = order_book_.cancel_order(order->order_id);
    EXPECT_TRUE(success);
    
    auto order_info = order_book_.get_order(order->order_id);
    EXPECT_TRUE(order_info);
    EXPECT_TRUE(order_info.value()->is_cancelled);
}

TEST_F(OrderBookTest, GetDepth) {
    // 添加买单
    order_book_.submit_order(
        "user1", Order::Side::BUY, Order::Type::LIMIT, 1000000000, 100000000
    );
    order_book_.submit_order(
        "user2", Order::Side::BUY, Order::Type::LIMIT, 999000000, 200000000
    );
    order_book_.submit_order(
        "user3", Order::Side::BUY, Order::Type::LIMIT, 998000000, 300000000
    );
    
    // 添加卖单
    order_book_.submit_order(
        "user4", Order::Side::SELL, Order::Type::LIMIT, 1001000000, 150000000
    );
    order_book_.submit_order(
        "user5", Order::Side::SELL, Order::Type::LIMIT, 1002000000, 250000000
    );
    order_book_.submit_order(
        "user6", Order::Side::SELL, Order::Type::LIMIT, 1003000000, 350000000
    );
    
    auto depth = order_book_.get_depth(2);
    
    EXPECT_EQ(depth.bids.size(), 2);
    EXPECT_EQ(depth.bids[0].price, 1000000000);
    EXPECT_EQ(depth.bids[0].quantity, 100000000);
    EXPECT_EQ(depth.bids[1].price, 999000000);
    EXPECT_EQ(depth.bids[1].quantity, 200000000);
    
    EXPECT_EQ(depth.asks.size(), 2);
    EXPECT_EQ(depth.asks[0].price, 1001000000);
    EXPECT_EQ(depth.asks[0].quantity, 150000000);
    EXPECT_EQ(depth.asks[1].price, 1002000000);
    EXPECT_EQ(depth.asks[1].quantity, 250000000);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
