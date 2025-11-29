#include <gtest/gtest.h>
#include "Trader.hpp"
#include <stdexcept>

class TraderTest : public ::testing::Test {
protected:
    void SetUp() override {
        trader = std::make_unique<Trader>(1, "Alice", 10000.0);
    }
    
    std::unique_ptr<Trader> trader;
};

TEST_F(TraderTest, ConstructorAndBasicGetters) {
    EXPECT_EQ(trader->getTraderId(), 1);
    EXPECT_EQ(trader->getName(), "Alice");
    EXPECT_EQ(trader->getCash(), 10000.0);
    EXPECT_EQ(trader->getPortfolioValue(), 10000.0);
    EXPECT_TRUE(trader->getPositions().empty());
}

TEST_F(TraderTest, CashManagement) {
    trader->addCash(5000.0);
    EXPECT_EQ(trader->getCash(), 15000.0);
    
    trader->addCash(-3000.0);
    EXPECT_EQ(trader->getCash(), 12000.0);
    
    EXPECT_THROW(trader->addCash(-15000.0), std::invalid_argument);
}

TEST_F(TraderTest, SufficientCashCheck) {
    EXPECT_TRUE(trader->hasSufficientCash(5000.0));
    EXPECT_TRUE(trader->hasSufficientCash(10000.0));
    EXPECT_FALSE(trader->hasSufficientCash(15000.0));
}

TEST_F(TraderTest, SufficientSharesCheck) {
    EXPECT_FALSE(trader->hasSufficientShares("AAPL", 10.0));
    
    // Simulate buying shares
    trader->onOrderFilled("AAPL", 50.0, 100.0, true);
    
    EXPECT_TRUE(trader->hasSufficientShares("AAPL", 30.0));
    EXPECT_TRUE(trader->hasSufficientShares("AAPL", 50.0));
    EXPECT_FALSE(trader->hasSufficientShares("AAPL", 60.0));
    EXPECT_FALSE(trader->hasSufficientShares("GOOGL", 10.0));
}

TEST_F(TraderTest, BuyOrderFilled) {
    double initialCash = trader->getCash();
    
    trader->onOrderFilled("AAPL", 50.0, 100.0, true);  // Buy 50 shares at $100 each
    
    EXPECT_EQ(trader->getCash(), initialCash - 5000.0);
    
    const auto& positions = trader->getPositions();
    EXPECT_EQ(positions.size(), 1);
    
    auto it = positions.find("AAPL");
    ASSERT_NE(it, positions.end());
    
    const Position& position = it->second;
    EXPECT_EQ(position.symbol, "AAPL");
    EXPECT_EQ(position.quantity, 50.0);
    EXPECT_EQ(position.averagePrice, 100.0);
}

TEST_F(TraderTest, SellOrderFilled) {
    // First buy some shares
    trader->onOrderFilled("AAPL", 100.0, 100.0, true);
    double cashAfterBuy = trader->getCash();
    
    // Then sell some
    trader->onOrderFilled("AAPL", 30.0, 110.0, false);  // Sell 30 shares at $110 each
    
    EXPECT_EQ(trader->getCash(), cashAfterBuy + 3300.0);
    
    const auto& positions = trader->getPositions();
    EXPECT_EQ(positions.size(), 1);
    
    auto it = positions.find("AAPL");
    ASSERT_NE(it, positions.end());
    
    const Position& position = it->second;
    EXPECT_EQ(position.quantity, 70.0);  // 100 - 30
    EXPECT_EQ(position.averagePrice, 100.0);  // Average price should remain the same
}

TEST_F(TraderTest, CompletePositionSale) {
    // Buy shares
    trader->onOrderFilled("AAPL", 50.0, 100.0, true);
    
    // Sell all shares
    trader->onOrderFilled("AAPL", 50.0, 110.0, false);
    
    // Position should be removed
    const auto& positions = trader->getPositions();
    EXPECT_TRUE(positions.empty());
}

TEST_F(TraderTest, AveragePriceCalculation) {
    // First purchase
    trader->onOrderFilled("AAPL", 50.0, 100.0, true);
    
    // Second purchase at different price
    trader->onOrderFilled("AAPL", 50.0, 120.0, true);
    
    const auto& positions = trader->getPositions();
    auto it = positions.find("AAPL");
    ASSERT_NE(it, positions.end());
    
    const Position& position = it->second;
    EXPECT_EQ(position.quantity, 100.0);
    EXPECT_EQ(position.averagePrice, 110.0);  // (50*100 + 50*120) / 100
}

TEST_F(TraderTest, InsufficientCashForBuy) {
    EXPECT_THROW(
        trader->onOrderFilled("AAPL", 200.0, 100.0, true),  // Costs $20,000 but only has $10,000
        std::runtime_error
    );
}

TEST_F(TraderTest, InsufficientSharesForSell) {
    EXPECT_THROW(
        trader->onOrderFilled("AAPL", 50.0, 100.0, false),  // Try to sell shares not owned
        std::runtime_error
    );
}

TEST_F(TraderTest, PortfolioValueCalculation) {
    double initialValue = trader->getPortfolioValue();
    
    // Buy some shares
    trader->onOrderFilled("AAPL", 50.0, 100.0, true);
    trader->onOrderFilled("GOOGL", 20.0, 200.0, true);
    
    // Portfolio value should include cash + positions at average price
    double expectedValue = trader->getCash() + (50.0 * 100.0) + (20.0 * 200.0);
    EXPECT_EQ(trader->getPortfolioValue(), expectedValue);
}

TEST_F(TraderTest, MultipleSymbolPositions) {
    trader->onOrderFilled("AAPL", 50.0, 100.0, true);
    trader->onOrderFilled("GOOGL", 20.0, 200.0, true);
    trader->onOrderFilled("MSFT", 30.0, 150.0, true);
    
    const auto& positions = trader->getPositions();
    EXPECT_EQ(positions.size(), 3);
    
    EXPECT_TRUE(positions.find("AAPL") != positions.end());
    EXPECT_TRUE(positions.find("GOOGL") != positions.end());
    EXPECT_TRUE(positions.find("MSFT") != positions.end());
}