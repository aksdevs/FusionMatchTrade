#include <gtest/gtest.h>
#include "MatchingEngine.hpp"
#include <memory>

class MatchingEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine = std::make_unique<MatchingEngine>();
        
        // Register test traders
        trader1 = std::make_shared<Trader>(1, "Alice", 100000.0);
        trader2 = std::make_shared<Trader>(2, "Bob", 100000.0);
        trader3 = std::make_shared<Trader>(3, "Charlie", 100000.0);
        
        engine->registerTrader(trader1);
        engine->registerTrader(trader2);
        engine->registerTrader(trader3);
    }
    
    std::unique_ptr<MatchingEngine> engine;
    std::shared_ptr<Trader> trader1;
    std::shared_ptr<Trader> trader2;
    std::shared_ptr<Trader> trader3;
};

TEST_F(MatchingEngineTest, RegisterAndGetTrader) {
    auto retrieved = engine->getTrader(1);
    EXPECT_EQ(retrieved, trader1);
    
    auto notFound = engine->getTrader(999);
    EXPECT_EQ(notFound, nullptr);
}

TEST_F(MatchingEngineTest, SubmitBuyOrder) {
    int orderId = engine->submitOrder(1, "AAPL", 100.0, 150.0, OrderSide::BUY);
    
    EXPECT_GT(orderId, 0);
    
    auto order = engine->getOrder(orderId);
    EXPECT_NE(order, nullptr);
    EXPECT_EQ(order->getTraderId(), 1);
    EXPECT_EQ(order->getSymbol(), "AAPL");
    EXPECT_EQ(order->getQuantity(), 100.0);
    EXPECT_EQ(order->getPrice(), 150.0);
    EXPECT_EQ(order->getSide(), OrderSide::BUY);
}

TEST_F(MatchingEngineTest, SubmitSellOrder) {
    // First buy some shares so we can sell them
    trader1->onOrderFilled("AAPL", 200.0, 100.0, true);
    
    int orderId = engine->submitOrder(1, "AAPL", 100.0, 150.0, OrderSide::SELL);
    
    EXPECT_GT(orderId, 0);
    
    auto order = engine->getOrder(orderId);
    EXPECT_NE(order, nullptr);
    EXPECT_EQ(order->getSide(), OrderSide::SELL);
}

TEST_F(MatchingEngineTest, InsufficientCashForBuyOrder) {
    EXPECT_THROW(
        engine->submitOrder(1, "AAPL", 1000.0, 200.0, OrderSide::BUY),  // Costs $200,000
        std::runtime_error
    );
}

TEST_F(MatchingEngineTest, InsufficientSharesForSellOrder) {
    EXPECT_THROW(
        engine->submitOrder(1, "AAPL", 100.0, 150.0, OrderSide::SELL),  // Don't own any shares
        std::runtime_error
    );
}

TEST_F(MatchingEngineTest, TraderNotFound) {
    EXPECT_THROW(
        engine->submitOrder(999, "AAPL", 100.0, 150.0, OrderSide::BUY),
        std::invalid_argument
    );
}

TEST_F(MatchingEngineTest, SimpleTradeExecution) {
    // Trader 2 places a sell order
    trader2->onOrderFilled("AAPL", 200.0, 100.0, true);  // Give trader2 some shares first
    int sellOrderId = engine->submitOrder(2, "AAPL", 100.0, 150.0, OrderSide::SELL);
    
    // Trader 1 places a matching buy order
    int buyOrderId = engine->submitOrder(1, "AAPL", 100.0, 150.0, OrderSide::BUY);
    
    // Both orders should be filled
    auto sellOrder = engine->getOrder(sellOrderId);
    auto buyOrder = engine->getOrder(buyOrderId);
    
    EXPECT_EQ(sellOrder->getStatus(), OrderStatus::FILLED);
    EXPECT_EQ(buyOrder->getStatus(), OrderStatus::FILLED);
    
    // Check trader positions
    EXPECT_TRUE(trader1->hasSufficientShares("AAPL", 100.0));
    EXPECT_TRUE(trader2->hasSufficientShares("AAPL", 100.0));  // 200 - 100 = 100 remaining
    
    // Check that a trade was recorded
    auto orderBook = engine->getOrderBook("AAPL");
    EXPECT_EQ(orderBook->getTradeCount(), 1);
}

TEST_F(MatchingEngineTest, PartialFillScenario) {
    // Trader 2 places a large sell order
    trader2->onOrderFilled("AAPL", 500.0, 100.0, true);  // Give trader2 shares first
    int sellOrderId = engine->submitOrder(2, "AAPL", 200.0, 150.0, OrderSide::SELL);
    
    // Trader 1 places a smaller buy order
    int buyOrderId = engine->submitOrder(1, "AAPL", 75.0, 150.0, OrderSide::BUY);
    
    auto sellOrder = engine->getOrder(sellOrderId);
    auto buyOrder = engine->getOrder(buyOrderId);
    
    EXPECT_EQ(buyOrder->getStatus(), OrderStatus::FILLED);
    EXPECT_EQ(sellOrder->getStatus(), OrderStatus::PARTIALLY_FILLED);
    EXPECT_EQ(sellOrder->getRemainingQuantity(), 125.0);
    
    // Check trader positions
    EXPECT_TRUE(trader1->hasSufficientShares("AAPL", 75.0));
}

TEST_F(MatchingEngineTest, PriorityOrderMatching) {
    // Give traders shares for selling
    trader2->onOrderFilled("AAPL", 300.0, 100.0, true);
    trader3->onOrderFilled("AAPL", 300.0, 100.0, true);
    
    // Trader 2 places sell order at $150
    int sellOrder1 = engine->submitOrder(2, "AAPL", 100.0, 150.0, OrderSide::SELL);
    
    // Trader 3 places sell order at $149 (better price)
    int sellOrder2 = engine->submitOrder(3, "AAPL", 100.0, 149.0, OrderSide::SELL);
    
    // Trader 1 places buy order that should match the better sell order first
    int buyOrderId = engine->submitOrder(1, "AAPL", 50.0, 155.0, OrderSide::BUY);
    
    auto orderBook = engine->getOrderBook("AAPL");
    const auto& trades = orderBook->getTrades();
    
    EXPECT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].price, 149.0);  // Should match the better (lower) sell price
    EXPECT_EQ(trades[0].sellTraderId, 3);  // Should match trader 3's order
}

TEST_F(MatchingEngineTest, CancelOrder) {
    int orderId = engine->submitOrder(1, "AAPL", 100.0, 150.0, OrderSide::BUY);
    
    bool cancelled = engine->cancelOrder(orderId);
    EXPECT_TRUE(cancelled);
    
    auto order = engine->getOrder(orderId);
    EXPECT_EQ(order, nullptr);  // Should be removed from engine's order map
    
    // Try to cancel non-existent order
    bool cancelledAgain = engine->cancelOrder(999);
    EXPECT_FALSE(cancelledAgain);
}

TEST_F(MatchingEngineTest, MarketDataMethods) {
    // Place some orders to create market data
    int buyOrderId = engine->submitOrder(1, "AAPL", 100.0, 148.0, OrderSide::BUY);
    
    trader2->onOrderFilled("AAPL", 300.0, 100.0, true);  // Give shares to trader2
    int sellOrderId = engine->submitOrder(2, "AAPL", 100.0, 152.0, OrderSide::SELL);
    
    EXPECT_EQ(engine->getBestBid("AAPL"), 148.0);
    EXPECT_EQ(engine->getBestAsk("AAPL"), 152.0);
    EXPECT_EQ(engine->getLastPrice("AAPL"), 0.0);  // No trades yet
    
    // Place a trade
    int tradeOrderId = engine->submitOrder(3, "AAPL", 50.0, 152.0, OrderSide::BUY);
    
    EXPECT_EQ(engine->getLastPrice("AAPL"), 152.0);
}

TEST_F(MatchingEngineTest, MultipleSymbols) {
    // Trade in AAPL
    int aaplOrderId = engine->submitOrder(1, "AAPL", 100.0, 150.0, OrderSide::BUY);
    
    // Trade in GOOGL
    int googlOrderId = engine->submitOrder(2, "GOOGL", 50.0, 2000.0, OrderSide::BUY);
    
    auto aaplOrderBook = engine->getOrderBook("AAPL");
    auto googlOrderBook = engine->getOrderBook("GOOGL");
    
    EXPECT_NE(aaplOrderBook, nullptr);
    EXPECT_NE(googlOrderBook, nullptr);
    EXPECT_NE(aaplOrderBook, googlOrderBook);
    
    EXPECT_EQ(aaplOrderBook->getSymbol(), "AAPL");
    EXPECT_EQ(googlOrderBook->getSymbol(), "GOOGL");
}

TEST_F(MatchingEngineTest, EngineStatistics) {
    // Initially no trades
    EXPECT_EQ(engine->getTotalTradeCount(), 0);
    EXPECT_EQ(engine->getTotalVolume(), 0.0);
    
    // Execute some trades
    trader2->onOrderFilled("AAPL", 300.0, 100.0, true);
    engine->submitOrder(2, "AAPL", 100.0, 150.0, OrderSide::SELL);
    engine->submitOrder(1, "AAPL", 100.0, 150.0, OrderSide::BUY);
    
    trader3->onOrderFilled("GOOGL", 300.0, 100.0, true);
    engine->submitOrder(3, "GOOGL", 50.0, 2000.0, OrderSide::SELL);
    engine->submitOrder(1, "GOOGL", 50.0, 2000.0, OrderSide::BUY);
    
    EXPECT_EQ(engine->getTotalTradeCount(), 2);
    EXPECT_EQ(engine->getTotalVolume(), 150.0);  // 100 + 50
}

TEST_F(MatchingEngineTest, GetAllTrades) {
    // Execute trades in multiple symbols
    trader2->onOrderFilled("AAPL", 300.0, 100.0, true);
    trader3->onOrderFilled("GOOGL", 300.0, 100.0, true);
    
    engine->submitOrder(2, "AAPL", 100.0, 150.0, OrderSide::SELL);
    engine->submitOrder(1, "AAPL", 100.0, 150.0, OrderSide::BUY);
    
    engine->submitOrder(3, "GOOGL", 50.0, 2000.0, OrderSide::SELL);
    engine->submitOrder(1, "GOOGL", 50.0, 2000.0, OrderSide::BUY);
    
    auto allTrades = engine->getAllTrades();
    EXPECT_EQ(allTrades.size(), 2);
    
    // Check that we have trades from both symbols
    bool hasApplTrade = false, hasGooglTrade = false;
    for (const auto& trade : allTrades) {
        if (trade.symbol == "AAPL") hasApplTrade = true;
        if (trade.symbol == "GOOGL") hasGooglTrade = true;
    }
    
    EXPECT_TRUE(hasApplTrade);
    EXPECT_TRUE(hasGooglTrade);
}