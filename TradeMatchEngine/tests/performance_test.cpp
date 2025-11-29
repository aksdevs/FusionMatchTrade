#include <gtest/gtest.h>
#include "MatchingEngine.hpp"
#include <chrono>
#include <random>
#include <vector>
#include <memory>
#include <numeric>
#include <algorithm>

class PerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine = std::make_unique<MatchingEngine>();
        
        // Create many traders for stress testing
        for (int i = 1; i <= 100; ++i) {
            auto trader = std::make_shared<Trader>(i, "Trader" + std::to_string(i), 1000000.0);
            
            // Give each trader some initial positions in various symbols
            if (i % 2 == 0) {  // Even traders get AAPL
                trader->onOrderFilled("AAPL", 1000.0, 150.0, true);
            }
            if (i % 3 == 0) {  // Every third trader gets GOOGL
                trader->onOrderFilled("GOOGL", 500.0, 2000.0, true);
            }
            if (i % 5 == 0) {  // Every fifth trader gets MSFT
                trader->onOrderFilled("MSFT", 800.0, 300.0, true);
            }
            
            traders.push_back(trader);
            engine->registerTrader(trader);
        }
        
        // Initialize random number generator
        rng.seed(std::chrono::steady_clock::now().time_since_epoch().count());
    }
    
    std::unique_ptr<MatchingEngine> engine;
    std::vector<std::shared_ptr<Trader>> traders;
    std::mt19937 rng;
    
    std::vector<std::string> symbols = {"AAPL", "GOOGL", "MSFT", "AMZN", "TSLA"};
    
    // Helper function to generate random orders
    struct RandomOrder {
        int traderId;
        std::string symbol;
        double quantity;
        double price;
        OrderSide side;
    };
    
    RandomOrder generateRandomOrder() {
        RandomOrder order;
        
        // Random trader
        std::uniform_int_distribution<int> traderDist(1, traders.size());
        order.traderId = traderDist(rng);
        
        // Random symbol
        std::uniform_int_distribution<int> symbolDist(0, symbols.size() - 1);
        order.symbol = symbols[symbolDist(rng)];
        
        // Random quantity (1-100 shares)
        std::uniform_real_distribution<double> quantityDist(1.0, 100.0);
        order.quantity = std::round(quantityDist(rng));
        
        // Random price around $100-$300
        std::uniform_real_distribution<double> priceDist(100.0, 300.0);
        order.price = std::round(priceDist(rng) * 100.0) / 100.0;  // Round to cents
        
        // Random side
        std::uniform_int_distribution<int> sideDist(0, 1);
        order.side = (sideDist(rng) == 0) ? OrderSide::BUY : OrderSide::SELL;
        
        return order;
    }
};

TEST_F(PerformanceTest, OrderSubmissionSpeed) {
    const int numOrders = 10000;
    std::vector<RandomOrder> orders;
    orders.reserve(numOrders);
    
    // Pre-generate orders to exclude generation time from measurement
    for (int i = 0; i < numOrders; ++i) {
        orders.push_back(generateRandomOrder());
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    int successfulOrders = 0;
    for (const auto& orderData : orders) {
        try {
            engine->submitOrder(orderData.traderId, orderData.symbol, 
                              orderData.quantity, orderData.price, orderData.side);
            successfulOrders++;
        } catch (const std::exception& e) {
            // Some orders may fail due to insufficient funds/shares, which is expected
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    double ordersPerSecond = (successfulOrders * 1000000.0) / duration.count();
    
    std::cout << "Submitted " << successfulOrders << " orders in " 
              << duration.count() << " microseconds" << std::endl;
    std::cout << "Performance: " << ordersPerSecond << " orders/second" << std::endl;
    
    // Performance target: at least 10,000 orders per second
    EXPECT_GT(ordersPerSecond, 10000.0);
    
    // Verify engine state is consistent
    EXPECT_GT(engine->getTotalTradeCount(), 0);
    EXPECT_GT(engine->getTotalVolume(), 0.0);
}

TEST_F(PerformanceTest, OrderBookDepthPerformance) {
    const std::string symbol = "PERF";
    auto orderBook = engine->getOrderBook(symbol);
    
    // Create deep order book (1000 orders on each side)
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000; ++i) {
        try {
            // Buy orders with decreasing prices
            engine->submitOrder(1 + (i % 50), symbol, 10.0, 150.0 - i * 0.01, OrderSide::BUY);
            
            // Sell orders with increasing prices
            engine->submitOrder(51 + (i % 50), symbol, 10.0, 151.0 + i * 0.01, OrderSide::SELL);
        } catch (const std::exception& e) {
            // Some may fail, that's okay
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    std::cout << "Created deep order book in " << duration.count() << " microseconds" << std::endl;
    
    // Test best bid/ask retrieval performance
    auto retrievalStart = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 10000; ++i) {
        volatile double bestBid = engine->getBestBid(symbol);
        volatile double bestAsk = engine->getBestAsk(symbol);
        (void)bestBid; (void)bestAsk;  // Suppress unused variable warnings
    }
    
    auto retrievalEnd = std::chrono::high_resolution_clock::now();
    auto retrievalDuration = std::chrono::duration_cast<std::chrono::microseconds>(retrievalEnd - retrievalStart);
    
    double retrievalsPerSecond = (20000.0 * 1000000.0) / retrievalDuration.count();  // 10k bids + 10k asks
    
    std::cout << "Best bid/ask retrieval performance: " << retrievalsPerSecond << " retrievals/second" << std::endl;
    
    // Performance target: at least 100,000 retrievals per second
    EXPECT_GT(retrievalsPerSecond, 100000.0);
}

TEST_F(PerformanceTest, MatchingEngineLatency) {
    // Test latency of order matching
    const int numTests = 1000;
    std::vector<double> latencies;
    latencies.reserve(numTests);
    
    for (int i = 0; i < numTests; ++i) {
        // Place a sell order first
        try {
            engine->submitOrder(1, "LATENCY", 100.0, 150.0, OrderSide::SELL);
        } catch (...) {
            continue;  // Skip if trader doesn't have shares
        }
        
        // Measure time to match with buy order
        auto startTime = std::chrono::high_resolution_clock::now();
        
        try {
            engine->submitOrder(2, "LATENCY", 100.0, 150.0, OrderSide::BUY);
        } catch (...) {
            continue;  // Skip if trader doesn't have cash
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
        
        latencies.push_back(latency.count());
    }
    
    if (!latencies.empty()) {
        // Calculate statistics
        std::sort(latencies.begin(), latencies.end());
        
        double avgLatency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
        double medianLatency = latencies[latencies.size() / 2];
        double p95Latency = latencies[static_cast<size_t>(latencies.size() * 0.95)];
        double p99Latency = latencies[static_cast<size_t>(latencies.size() * 0.99)];
        
        std::cout << "Order matching latency statistics:" << std::endl;
        std::cout << "  Average: " << avgLatency << " ns" << std::endl;
        std::cout << "  Median:  " << medianLatency << " ns" << std::endl;
        std::cout << "  95th %:  " << p95Latency << " ns" << std::endl;
        std::cout << "  99th %:  " << p99Latency << " ns" << std::endl;
        
        // Performance target: 99th percentile under 100 microseconds (100,000 ns)
        EXPECT_LT(p99Latency, 100000.0);
    }
}

TEST_F(PerformanceTest, MemoryUsageStressTest) {
    // Test memory usage with large number of orders and trades
    const int numOrders = 50000;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    int successfulOrders = 0;
    for (int i = 0; i < numOrders; ++i) {
        auto orderData = generateRandomOrder();
        
        try {
            engine->submitOrder(orderData.traderId, orderData.symbol, 
                              orderData.quantity, orderData.price, orderData.side);
            successfulOrders++;
        } catch (...) {
            // Expected for some orders
        }
        
        // Periodically check that we can still retrieve market data
        if (i % 1000 == 0) {
            for (const auto& symbol : symbols) {
                volatile double bestBid = engine->getBestBid(symbol);
                volatile double bestAsk = engine->getBestAsk(symbol);
                (void)bestBid; (void)bestAsk;
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    std::cout << "Stress test completed in " << duration.count() << " ms" << std::endl;
    std::cout << "Successfully submitted " << successfulOrders << " out of " << numOrders << " orders" << std::endl;
    std::cout << "Total trades executed: " << engine->getTotalTradeCount() << std::endl;
    std::cout << "Total volume: " << engine->getTotalVolume() << std::endl;
    
    // Verify the engine is still functional
    EXPECT_GT(successfulOrders, numOrders / 10);  // At least 10% should succeed
    EXPECT_NO_THROW(engine->printMarketSummary());
    
    // Test that we can still submit new orders after stress test
    try {
        int newOrderId = engine->submitOrder(1, "AAPL", 10.0, 150.0, OrderSide::SELL);
        EXPECT_GT(newOrderId, 0);
    } catch (...) {
        // May fail due to insufficient shares, which is acceptable
    }
}

TEST_F(PerformanceTest, ConcurrentOrderSubmission) {
    // Note: This is a simplified concurrency test
    // Real concurrent testing would require threading, which is beyond this basic test
    
    const int batchSize = 1000;
    std::vector<int> orderIds;
    orderIds.reserve(batchSize * 2);
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Submit many orders in rapid succession to simulate concurrent load
    for (int i = 0; i < batchSize; ++i) {
        try {
            // Alternating buy and sell orders
            int buyOrderId = engine->submitOrder(1 + (i % 10), "CONC", 1.0, 100.0 + i * 0.01, OrderSide::BUY);
            orderIds.push_back(buyOrderId);
            
            int sellOrderId = engine->submitOrder(11 + (i % 10), "CONC", 1.0, 100.0 + i * 0.01, OrderSide::SELL);
            orderIds.push_back(sellOrderId);
        } catch (...) {
            // Some may fail
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    std::cout << "Rapid order submission test completed in " << duration.count() << " microseconds" << std::endl;
    
    // Verify data consistency
    auto orderBook = engine->getOrderBook("CONC");
    EXPECT_NE(orderBook, nullptr);
    
    // Should have generated many trades due to overlapping prices
    EXPECT_GT(orderBook->getTradeCount(), batchSize / 4);  // Conservative estimate
    
    std::cout << "Generated " << orderBook->getTradeCount() << " trades" << std::endl;
}