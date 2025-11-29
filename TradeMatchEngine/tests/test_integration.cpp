#include <gtest/gtest.h>
#include "MatchingEngine.hpp"
#include "Trader.hpp"
#include <memory>
#include <vector>

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine = std::make_unique<MatchingEngine>();
        
        // Create multiple traders with different initial conditions
        alice = std::make_shared<Trader>(1, "Alice", 50000.0);
        bob = std::make_shared<Trader>(2, "Bob", 75000.0);
        charlie = std::make_shared<Trader>(3, "Charlie", 100000.0);
        diana = std::make_shared<Trader>(4, "Diana", 60000.0);
        
        // Register all traders
        engine->registerTrader(alice);
        engine->registerTrader(bob);
        engine->registerTrader(charlie);
        engine->registerTrader(diana);
        
        // Give some traders initial positions
        alice->onOrderFilled("AAPL", 100.0, 140.0, true);    // Alice has 100 AAPL at avg $140
        bob->onOrderFilled("GOOGL", 50.0, 1800.0, true);     // Bob has 50 GOOGL at avg $1800
        charlie->onOrderFilled("AAPL", 200.0, 145.0, true);  // Charlie has 200 AAPL at avg $145
    }
    
    std::unique_ptr<MatchingEngine> engine;
    std::shared_ptr<Trader> alice;
    std::shared_ptr<Trader> bob;
    std::shared_ptr<Trader> charlie;
    std::shared_ptr<Trader> diana;
};

TEST_F(IntegrationTest, ComplexTradingScenario) {
    // Day trading scenario with multiple participants
    
    // Morning: Alice wants to sell some AAPL
    int aliceSellOrder = engine->submitOrder(1, "AAPL", 50.0, 150.0, OrderSide::SELL);
    
    // Diana sees the opportunity and buys
    int dianaBuyOrder = engine->submitOrder(4, "AAPL", 50.0, 150.0, OrderSide::BUY);
    
    // Verify the trade
    auto orderBook = engine->getOrderBook("AAPL");
    EXPECT_EQ(orderBook->getTradeCount(), 1);
    EXPECT_EQ(orderBook->getLastTradePrice(), 150.0);
    
    // Check positions after trade
    EXPECT_TRUE(alice->hasSufficientShares("AAPL", 50.0));   // Alice has 50 left (100 - 50)
    EXPECT_TRUE(diana->hasSufficientShares("AAPL", 50.0));   // Diana now owns 50
    
    // Afternoon: Charlie wants to buy more AAPL but at a lower price
    int charlieBuyOrder = engine->submitOrder(3, "AAPL", 75.0, 148.0, OrderSide::BUY);
    
    // Alice decides to sell more at market
    int aliceSecondSell = engine->submitOrder(1, "AAPL", 50.0, 148.0, OrderSide::SELL);
    
    // This should create another trade
    EXPECT_EQ(orderBook->getTradeCount(), 2);
    EXPECT_EQ(orderBook->getLastTradePrice(), 148.0);
    
    // Charlie should have partial fill
    auto charlieOrder = engine->getOrder(charlieBuyOrder);
    EXPECT_EQ(charlieOrder->getFilledQuantity(), 50.0);
    EXPECT_EQ(charlieOrder->getRemainingQuantity(), 25.0);
    EXPECT_EQ(charlieOrder->getStatus(), OrderStatus::PARTIALLY_FILLED);
    
    // Alice should be out of AAPL now
    EXPECT_FALSE(alice->hasSufficientShares("AAPL", 1.0));
}

TEST_F(IntegrationTest, MultiSymbolTrading) {
    // Complex scenario involving multiple symbols
    
    // Bob wants to diversify - sell some GOOGL to buy AAPL
    int bobSellGoogl = engine->submitOrder(2, "GOOGL", 25.0, 2000.0, OrderSide::SELL);
    
    // Diana wants GOOGL exposure
    int dianaBuyGoogl = engine->submitOrder(4, "GOOGL", 25.0, 2000.0, OrderSide::BUY);
    
    // Now Bob uses proceeds to buy AAPL from Charlie
    int bobBuyAapl = engine->submitOrder(2, "AAPL", 100.0, 148.0, OrderSide::BUY);
    int charlieSellAapl = engine->submitOrder(3, "AAPL", 100.0, 148.0, OrderSide::SELL);
    
    // Check final positions
    auto aaplBook = engine->getOrderBook("AAPL");
    auto googlBook = engine->getOrderBook("GOOGL");
    
    EXPECT_EQ(aaplBook->getTradeCount(), 1);
    EXPECT_EQ(googlBook->getTradeCount(), 1);
    
    // Bob should now have both AAPL and GOOGL
    EXPECT_TRUE(bob->hasSufficientShares("AAPL", 100.0));
    EXPECT_TRUE(bob->hasSufficientShares("GOOGL", 25.0));  // 50 - 25 = 25 remaining
    
    // Diana should have GOOGL
    EXPECT_TRUE(diana->hasSufficientShares("GOOGL", 25.0));
    
    // Charlie should have less AAPL
    EXPECT_TRUE(charlie->hasSufficientShares("AAPL", 100.0));  // 200 - 100 = 100 remaining
}

TEST_F(IntegrationTest, OrderBookDepthAndLiquidity) {
    // Create a deep order book with multiple price levels
    
    // Charlie places multiple sell orders at different prices
    engine->submitOrder(3, "AAPL", 50.0, 150.0, OrderSide::SELL);
    engine->submitOrder(3, "AAPL", 30.0, 151.0, OrderSide::SELL);
    engine->submitOrder(3, "AAPL", 40.0, 152.0, OrderSide::SELL);
    
    auto orderBook = engine->getOrderBook("AAPL");
    EXPECT_EQ(orderBook->getAskDepth(), 3);
    EXPECT_EQ(orderBook->getBestAskPrice(), 150.0);
    
    // Diana places buy orders at different levels
    engine->submitOrder(4, "AAPL", 25.0, 149.0, OrderSide::BUY);
    engine->submitOrder(4, "AAPL", 35.0, 148.0, OrderSide::BUY);
    
    EXPECT_EQ(orderBook->getBidDepth(), 2);
    EXPECT_EQ(orderBook->getBestBidPrice(), 149.0);
    EXPECT_EQ(orderBook->getSpread(), 1.0);  // 150 - 149
    
    // Alice places a large market sweep order
    int aliceLargeOrder = engine->submitOrder(1, "AAPL", 100.0, 155.0, OrderSide::BUY);
    
    // This should match multiple sell orders
    EXPECT_EQ(orderBook->getTradeCount(), 3);  // Should match all three sell levels
    
    auto aliceOrder = engine->getOrder(aliceLargeOrder);
    EXPECT_EQ(aliceOrder->getFilledQuantity(), 100.0);  // 50 + 30 + 20 (partial from third level)
    EXPECT_EQ(aliceOrder->getStatus(), OrderStatus::FILLED);
}

TEST_F(IntegrationTest, OrderCancellationScenario) {
    // Test realistic order cancellation scenario
    
    // Charlie places a large sell order
    int charlieOrder = engine->submitOrder(3, "AAPL", 100.0, 155.0, OrderSide::SELL);
    
    // Diana places a buy order that doesn't match
    int dianaOrder = engine->submitOrder(4, "AAPL", 50.0, 150.0, OrderSide::BUY);
    
    auto orderBook = engine->getOrderBook("AAPL");
    EXPECT_EQ(orderBook->getBidDepth(), 1);
    EXPECT_EQ(orderBook->getAskDepth(), 1);
    EXPECT_EQ(orderBook->getTradeCount(), 0);  // No trades yet
    
    // Charlie changes his mind and cancels the order
    bool cancelled = engine->cancelOrder(charlieOrder);
    EXPECT_TRUE(cancelled);
    
    EXPECT_EQ(orderBook->getAskDepth(), 0);  // Should be cleaned up
    
    // Alice now places a sell order at the bid price
    int aliceOrder = engine->submitOrder(1, "AAPL", 25.0, 150.0, OrderSide::SELL);
    
    // This should match with Diana's buy order
    EXPECT_EQ(orderBook->getTradeCount(), 1);
    
    auto dianaOrderFinal = engine->getOrder(dianaOrder);
    EXPECT_EQ(dianaOrderFinal->getStatus(), OrderStatus::PARTIALLY_FILLED);
    EXPECT_EQ(dianaOrderFinal->getRemainingQuantity(), 25.0);
}

TEST_F(IntegrationTest, PortfolioTrackingThroughTrades) {
    // Test that portfolio values are correctly maintained through trades
    
    double aliceInitialValue = alice->getPortfolioValue();
    double charlieInitialValue = charlie->getPortfolioValue();
    
    // Execute a trade between Alice and Charlie
    int aliceSell = engine->submitOrder(1, "AAPL", 50.0, 152.0, OrderSide::SELL);
    int charlieBuy = engine->submitOrder(3, "AAPL", 50.0, 152.0, OrderSide::BUY);
    
    // Portfolio values should change but total value should be conserved (minus any fees)
    double aliceFinalValue = alice->getPortfolioValue();
    double charlieFinalValue = charlie->getPortfolioValue();
    
    // Alice should have more cash, less stock
    EXPECT_GT(alice->getCash(), 50000.0 - 100.0 * 140.0);  // Original cash minus initial AAPL purchase
    EXPECT_TRUE(alice->hasSufficientShares("AAPL", 50.0));  // 100 - 50 = 50 remaining
    
    // Charlie should have less cash, more stock
    EXPECT_LT(charlie->getCash(), 100000.0 - 200.0 * 145.0);  // Original cash minus initial AAPL purchase minus new purchase
    EXPECT_TRUE(charlie->hasSufficientShares("AAPL", 250.0));  // 200 + 50 = 250 total
    
    // The difference in portfolio values should equal the trade amount
    double tradeValue = 50.0 * 152.0;  // 50 shares at $152
    
    // Alice gained cash and lost stock value
    double aliceGain = (alice->getCash() - (50000.0 - 100.0 * 140.0)) - (50.0 * 140.0);  // Cash gain minus stock value lost
    
    // This is approximately the profit/loss from the price difference
    EXPECT_GT(aliceGain, 0);  // Alice sold at $152, her average cost was $140
}

TEST_F(IntegrationTest, MarketMakingScenario) {
    // Simulate a market maker providing liquidity
    
    // Diana acts as market maker, placing both buy and sell orders
    engine->submitOrder(4, "MSFT", 100.0, 299.0, OrderSide::BUY);   // Bid
    engine->submitOrder(4, "MSFT", 100.0, 301.0, OrderSide::SELL);  // Ask
    
    auto msftBook = engine->getOrderBook("MSFT");
    EXPECT_EQ(msftBook->getBestBidPrice(), 299.0);
    EXPECT_EQ(msftBook->getBestAskPrice(), 301.0);
    EXPECT_EQ(msftBook->getSpread(), 2.0);
    
    // Alice hits the ask
    engine->submitOrder(1, "MSFT", 50.0, 301.0, OrderSide::BUY);
    
    // Bob lifts the bid
    // First give Diana some MSFT shares
    diana->onOrderFilled("MSFT", 150.0, 300.0, true);
    engine->submitOrder(2, "MSFT", 30.0, 299.0, OrderSide::SELL);
    
    EXPECT_EQ(msftBook->getTradeCount(), 2);
    
    // Diana should have made spread on both trades
    // She sold at 301 and bought at 299, making $2 per share on the round trip
}

TEST_F(IntegrationTest, StressTestWithManyOrders) {
    // Submit many orders quickly to test performance and correctness
    
    std::vector<int> orderIds;
    
    // Give traders more shares for stress testing
    alice->onOrderFilled("STRESS", 1000.0, 100.0, true);
    bob->onOrderFilled("STRESS", 1000.0, 100.0, true);
    charlie->onOrderFilled("STRESS", 1000.0, 100.0, true);
    
    // Submit 100 orders alternating between buy and sell
    for (int i = 0; i < 50; ++i) {
        // Sell orders from different traders
        orderIds.push_back(engine->submitOrder(1, "STRESS", 10.0, 100.0 + i * 0.1, OrderSide::SELL));
        orderIds.push_back(engine->submitOrder(2, "STRESS", 10.0, 100.0 + i * 0.1, OrderSide::SELL));
        
        // Buy orders from other traders
        orderIds.push_back(engine->submitOrder(4, "STRESS", 15.0, 100.0 + i * 0.1, OrderSide::BUY));
    }
    
    auto stressBook = engine->getOrderBook("STRESS");
    
    // Should have generated many trades
    EXPECT_GT(stressBook->getTradeCount(), 40);  // Most orders should have matched
    
    // Order book should still be functional
    EXPECT_GT(stressBook->getBestBidPrice(), 0.0);
    EXPECT_GT(stressBook->getBestAskPrice(), 0.0);
    
    // Engine statistics should be consistent
    EXPECT_EQ(engine->getTotalTradeCount(), stressBook->getTradeCount());
}