#include <gtest/gtest.h>
#include "OrderBook.hpp"
#include <memory>

class OrderBookTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        orderBook = std::make_unique<OrderBook>("AAPL");
    }

    std::unique_ptr<OrderBook> orderBook;

    std::shared_ptr<Order> createBuyOrder(int id, double quantity, double price)
    {
        return std::make_shared<Order>(id, 100 + id, "AAPL", quantity, price, OrderSide::BUY);
    }

    std::shared_ptr<Order> createSellOrder(int id, double quantity, double price)
    {
        return std::make_shared<Order>(id, 200 + id, "AAPL", quantity, price, OrderSide::SELL);
    }
};

TEST_F(OrderBookTest, Constructor)
{
    EXPECT_EQ(orderBook->getSymbol(), "AAPL");
    EXPECT_EQ(orderBook->getBestBidPrice(), 0.0);
    EXPECT_EQ(orderBook->getBestAskPrice(), 0.0);
    EXPECT_EQ(orderBook->getSpread(), 0.0);
    EXPECT_EQ(orderBook->getBidDepth(), 0);
    EXPECT_EQ(orderBook->getAskDepth(), 0);
    EXPECT_TRUE(orderBook->getTrades().empty());
}

TEST_F(OrderBookTest, AddSingleBuyOrder)
{
    auto order = createBuyOrder(1, 100.0, 150.0);
    orderBook->addOrder(order);

    EXPECT_EQ(orderBook->getBestBidPrice(), 150.0);
    EXPECT_EQ(orderBook->getBestAskPrice(), 0.0);
    EXPECT_EQ(orderBook->getBidDepth(), 1);
    EXPECT_EQ(orderBook->getAskDepth(), 0);
    EXPECT_TRUE(orderBook->getTrades().empty());
}

TEST_F(OrderBookTest, AddSingleSellOrder)
{
    auto order = createSellOrder(1, 100.0, 160.0);
    orderBook->addOrder(order);

    EXPECT_EQ(orderBook->getBestBidPrice(), 0.0);
    EXPECT_EQ(orderBook->getBestAskPrice(), 160.0);
    EXPECT_EQ(orderBook->getBidDepth(), 0);
    EXPECT_EQ(orderBook->getAskDepth(), 1);
    EXPECT_TRUE(orderBook->getTrades().empty());
}

TEST_F(OrderBookTest, AddMultipleBuyOrders)
{
    auto order1 = createBuyOrder(1, 100.0, 150.0);
    auto order2 = createBuyOrder(2, 50.0, 155.0); // Higher price
    auto order3 = createBuyOrder(3, 75.0, 145.0); // Lower price

    orderBook->addOrder(order1);
    orderBook->addOrder(order2);
    orderBook->addOrder(order3);

    // DEBUG: print order book after adding buy orders
    std::cout << "DEBUG: After AddMultipleBuyOrders" << std::endl;
    orderBook->printOrderBook();

    EXPECT_EQ(orderBook->getBestBidPrice(), 155.0); // Highest bid
    EXPECT_EQ(orderBook->getBidDepth(), 3);
}

TEST_F(OrderBookTest, AddMultipleSellOrders)
{
    auto order1 = createSellOrder(1, 100.0, 160.0);
    auto order2 = createSellOrder(2, 50.0, 155.0); // Lower price
    auto order3 = createSellOrder(3, 75.0, 165.0); // Higher price

    orderBook->addOrder(order1);
    orderBook->addOrder(order2);
    orderBook->addOrder(order3);

    // DEBUG: print order book after adding sell orders
    std::cout << "DEBUG: After AddMultipleSellOrders" << std::endl;
    orderBook->printOrderBook();

    EXPECT_EQ(orderBook->getBestAskPrice(), 155.0); // Lowest ask
    EXPECT_EQ(orderBook->getAskDepth(), 3);
}

TEST_F(OrderBookTest, SimpleMatchBuyAgainstSell)
{
    // Add a sell order first
    auto sellOrder = createSellOrder(1, 100.0, 150.0);
    orderBook->addOrder(sellOrder);

    EXPECT_TRUE(orderBook->getTrades().empty());
    EXPECT_EQ(orderBook->getAskDepth(), 1);

    // Add a buy order that matches
    auto buyOrder = createBuyOrder(2, 50.0, 150.0);
    orderBook->addOrder(buyOrder);

    // Should create a trade
    const auto &trades = orderBook->getTrades();
    EXPECT_EQ(trades.size(), 1);

    const Trade &trade = trades[0];
    EXPECT_EQ(trade.buyOrderId, 2);
    EXPECT_EQ(trade.sellOrderId, 1);
    EXPECT_EQ(trade.quantity, 50.0);
    EXPECT_EQ(trade.price, 150.0); // Sell order price (price improvement for buyer)

    // Check order states
    EXPECT_EQ(buyOrder->getStatus(), OrderStatus::FILLED);
    EXPECT_EQ(sellOrder->getStatus(), OrderStatus::PARTIALLY_FILLED);
    EXPECT_EQ(sellOrder->getRemainingQuantity(), 50.0);

    // Order book should still have the remaining sell order
    EXPECT_EQ(orderBook->getAskDepth(), 1);
    EXPECT_EQ(orderBook->getBestAskPrice(), 150.0);
}

TEST_F(OrderBookTest, SimpleMatchSellAgainstBuy)
{
    // Add a buy order first
    auto buyOrder = createBuyOrder(1, 100.0, 150.0);
    orderBook->addOrder(buyOrder);

    EXPECT_TRUE(orderBook->getTrades().empty());
    EXPECT_EQ(orderBook->getBidDepth(), 1);

    // Add a sell order that matches
    auto sellOrder = createSellOrder(2, 75.0, 150.0);
    orderBook->addOrder(sellOrder);

    // Should create a trade
    const auto &trades = orderBook->getTrades();
    EXPECT_EQ(trades.size(), 1);

    const Trade &trade = trades[0];
    EXPECT_EQ(trade.buyOrderId, 1);
    EXPECT_EQ(trade.sellOrderId, 2);
    EXPECT_EQ(trade.quantity, 75.0);
    EXPECT_EQ(trade.price, 150.0); // Buy order price (price improvement for seller)

    // Check order states
    EXPECT_EQ(sellOrder->getStatus(), OrderStatus::FILLED);
    EXPECT_EQ(buyOrder->getStatus(), OrderStatus::PARTIALLY_FILLED);
    EXPECT_EQ(buyOrder->getRemainingQuantity(), 25.0);
}

TEST_F(OrderBookTest, PriceImprovementForBuyer)
{
    // Sell order at $150
    auto sellOrder = createSellOrder(1, 100.0, 150.0);
    orderBook->addOrder(sellOrder);

    // Buy order willing to pay $155
    auto buyOrder = createBuyOrder(2, 50.0, 155.0);
    orderBook->addOrder(buyOrder);

    const auto &trades = orderBook->getTrades();
    EXPECT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].price, 150.0); // Buyer gets price improvement
}

TEST_F(OrderBookTest, PriceImprovementForSeller)
{
    // Buy order at $155
    auto buyOrder = createBuyOrder(1, 100.0, 155.0);
    orderBook->addOrder(buyOrder);

    // Sell order willing to accept $150
    auto sellOrder = createSellOrder(2, 50.0, 150.0);
    orderBook->addOrder(sellOrder);

    const auto &trades = orderBook->getTrades();
    EXPECT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].price, 155.0); // Seller gets price improvement
}

TEST_F(OrderBookTest, NoMatchDueToPrice)
{
    // Buy order at $140
    auto buyOrder = createBuyOrder(1, 100.0, 140.0);
    orderBook->addOrder(buyOrder);

    // Sell order at $150 - no match
    auto sellOrder = createSellOrder(2, 50.0, 150.0);
    orderBook->addOrder(sellOrder);

    EXPECT_TRUE(orderBook->getTrades().empty());
    EXPECT_EQ(orderBook->getBidDepth(), 1);
    EXPECT_EQ(orderBook->getAskDepth(), 1);
    EXPECT_EQ(orderBook->getSpread(), 10.0); // $150 - $140
}

TEST_F(OrderBookTest, CompleteMatch)
{
    // Sell order
    auto sellOrder = createSellOrder(1, 100.0, 150.0);
    orderBook->addOrder(sellOrder);

    // Buy order for exact same quantity
    auto buyOrder = createBuyOrder(2, 100.0, 150.0);
    orderBook->addOrder(buyOrder);

    const auto &trades = orderBook->getTrades();
    EXPECT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].quantity, 100.0);

    EXPECT_EQ(buyOrder->getStatus(), OrderStatus::FILLED);
    EXPECT_EQ(sellOrder->getStatus(), OrderStatus::FILLED);

    // Order book should be empty
    EXPECT_EQ(orderBook->getBidDepth(), 0);
    EXPECT_EQ(orderBook->getAskDepth(), 0);
}

TEST_F(OrderBookTest, MultipleMatches)
{
    // Add multiple sell orders
    auto sell1 = createSellOrder(1, 30.0, 150.0);
    auto sell2 = createSellOrder(2, 40.0, 151.0);
    auto sell3 = createSellOrder(3, 50.0, 152.0);

    orderBook->addOrder(sell1);
    orderBook->addOrder(sell2);
    orderBook->addOrder(sell3);

    // Add large buy order that should match multiple sells
    auto buyOrder = createBuyOrder(4, 100.0, 155.0);
    orderBook->addOrder(buyOrder);

    const auto &trades = orderBook->getTrades();
    // DEBUG: print order book and trades after matching
    std::cout << "DEBUG: After MultipleMatches - Order Book" << std::endl;
    orderBook->printOrderBook();
    std::cout << "DEBUG: Trades (count=" << trades.size() << ")" << std::endl;
    for (size_t i = 0; i < trades.size(); ++i)
    {
        const auto &t = trades[i];
        std::cout << "  Trade[" << i << "] price=" << t.price << " qty=" << t.quantity << " buyId=" << t.buyOrderId << " sellId=" << t.sellOrderId << std::endl;
    }

    EXPECT_EQ(trades.size(), 3); // Should match all three sell orders

    // Check trade details
    EXPECT_EQ(trades[0].price, 150.0);
    EXPECT_EQ(trades[0].quantity, 30.0);

    EXPECT_EQ(trades[1].price, 151.0);
    EXPECT_EQ(trades[1].quantity, 40.0);
    EXPECT_EQ(trades[2].price, 152.0);
    EXPECT_EQ(trades[2].quantity, 30.0); // Only part of the buy order

    EXPECT_EQ(buyOrder->getStatus(), OrderStatus::FILLED);
    EXPECT_EQ(buyOrder->getRemainingQuantity(), 0.0);
}

TEST_F(OrderBookTest, CancelOrder)
{
    auto order = createBuyOrder(1, 100.0, 150.0);
    orderBook->addOrder(order);

    EXPECT_EQ(orderBook->getBidDepth(), 1);

    bool cancelled = orderBook->cancelOrder(1);
    EXPECT_TRUE(cancelled);
    EXPECT_EQ(order->getStatus(), OrderStatus::CANCELLED);

    // Order book should clean up cancelled orders
    EXPECT_EQ(orderBook->getBidDepth(), 0);

    // Try to cancel non-existent order
    bool cancelledAgain = orderBook->cancelOrder(999);
    EXPECT_FALSE(cancelledAgain);
}

TEST_F(OrderBookTest, GetOrderById)
{
    auto order = createBuyOrder(1, 100.0, 150.0);
    orderBook->addOrder(order);

    auto retrieved = orderBook->getOrder(1);
    EXPECT_EQ(retrieved, order);

    auto notFound = orderBook->getOrder(999);
    EXPECT_EQ(notFound, nullptr);
}

TEST_F(OrderBookTest, WrongSymbol)
{
    auto order = std::make_shared<Order>(1, 100, "GOOGL", 100.0, 150.0, OrderSide::BUY);

    EXPECT_THROW(orderBook->addOrder(order), std::invalid_argument);
}
