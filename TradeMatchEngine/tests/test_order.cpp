#include <gtest/gtest.h>
#include "Order.hpp"
#include <stdexcept>
#include <thread>
#include <chrono>

class OrderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test data setup
    }
};

TEST_F(OrderTest, ConstructorValidInput) {
    Order order(1, 100, "AAPL", 50.0, 150.0, OrderSide::BUY);
    
    EXPECT_EQ(order.getOrderId(), 1);
    EXPECT_EQ(order.getTraderId(), 100);
    EXPECT_EQ(order.getSymbol(), "AAPL");
    EXPECT_EQ(order.getQuantity(), 50.0);
    EXPECT_EQ(order.getPrice(), 150.0);
    EXPECT_EQ(order.getSide(), OrderSide::BUY);
    EXPECT_EQ(order.getType(), OrderType::LIMIT);
    EXPECT_EQ(order.getStatus(), OrderStatus::PENDING);
    EXPECT_EQ(order.getFilledQuantity(), 0.0);
    EXPECT_EQ(order.getRemainingQuantity(), 50.0);
}

TEST_F(OrderTest, ConstructorInvalidQuantity) {
    EXPECT_THROW(
        Order(1, 100, "AAPL", -50.0, 150.0, OrderSide::BUY),
        std::invalid_argument
    );
    
    EXPECT_THROW(
        Order(1, 100, "AAPL", 0.0, 150.0, OrderSide::BUY),
        std::invalid_argument
    );
}

TEST_F(OrderTest, ConstructorInvalidPrice) {
    EXPECT_THROW(
        Order(1, 100, "AAPL", 50.0, -150.0, OrderSide::BUY),
        std::invalid_argument
    );
    
    EXPECT_THROW(
        Order(1, 100, "AAPL", 50.0, 0.0, OrderSide::BUY),
        std::invalid_argument
    );
}

TEST_F(OrderTest, AddFillPartial) {
    Order order(1, 100, "AAPL", 100.0, 150.0, OrderSide::BUY);
    
    order.addFill(30.0);
    
    EXPECT_EQ(order.getFilledQuantity(), 30.0);
    EXPECT_EQ(order.getRemainingQuantity(), 70.0);
    EXPECT_EQ(order.getStatus(), OrderStatus::PARTIALLY_FILLED);
    EXPECT_FALSE(order.isComplete());
}

TEST_F(OrderTest, AddFillComplete) {
    Order order(1, 100, "AAPL", 50.0, 150.0, OrderSide::BUY);
    
    order.addFill(50.0);
    
    EXPECT_EQ(order.getFilledQuantity(), 50.0);
    EXPECT_EQ(order.getRemainingQuantity(), 0.0);
    EXPECT_EQ(order.getStatus(), OrderStatus::FILLED);
    EXPECT_TRUE(order.isComplete());
}

TEST_F(OrderTest, AddFillInvalidQuantity) {
    Order order(1, 100, "AAPL", 50.0, 150.0, OrderSide::BUY);
    
    EXPECT_THROW(order.addFill(-10.0), std::invalid_argument);
    EXPECT_THROW(order.addFill(0.0), std::invalid_argument);
}

TEST_F(OrderTest, AddFillExceedsQuantity) {
    Order order(1, 100, "AAPL", 50.0, 150.0, OrderSide::BUY);
    
    EXPECT_THROW(order.addFill(60.0), std::invalid_argument);
}

TEST_F(OrderTest, AddMultipleFills) {
    Order order(1, 100, "AAPL", 100.0, 150.0, OrderSide::BUY);
    
    order.addFill(30.0);
    order.addFill(40.0);
    
    EXPECT_EQ(order.getFilledQuantity(), 70.0);
    EXPECT_EQ(order.getRemainingQuantity(), 30.0);
    EXPECT_EQ(order.getStatus(), OrderStatus::PARTIALLY_FILLED);
    
    order.addFill(30.0);
    
    EXPECT_EQ(order.getFilledQuantity(), 100.0);
    EXPECT_EQ(order.getRemainingQuantity(), 0.0);
    EXPECT_EQ(order.getStatus(), OrderStatus::FILLED);
    EXPECT_TRUE(order.isComplete());
}

TEST_F(OrderTest, OrderSideHelpers) {
    Order buyOrder(1, 100, "AAPL", 50.0, 150.0, OrderSide::BUY);
    Order sellOrder(2, 100, "AAPL", 50.0, 150.0, OrderSide::SELL);
    
    EXPECT_TRUE(buyOrder.isBuy());
    EXPECT_FALSE(buyOrder.isSell());
    
    EXPECT_FALSE(sellOrder.isBuy());
    EXPECT_TRUE(sellOrder.isSell());
}

TEST_F(OrderTest, PriorityComparison) {
    // Create orders with different prices and timestamps
    Order buyOrder1(1, 100, "AAPL", 50.0, 150.0, OrderSide::BUY);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    Order buyOrder2(2, 100, "AAPL", 50.0, 151.0, OrderSide::BUY);  // Higher price
    
    Order sellOrder1(3, 100, "AAPL", 50.0, 149.0, OrderSide::SELL);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    Order sellOrder2(4, 100, "AAPL", 50.0, 148.0, OrderSide::SELL);  // Lower price
    
    // For buy orders, higher price should have higher priority (lower in comparison)
    EXPECT_TRUE(buyOrder1 < buyOrder2);
    
    // For sell orders, lower price should have higher priority (lower in comparison)
    EXPECT_TRUE(sellOrder1 < sellOrder2);
}