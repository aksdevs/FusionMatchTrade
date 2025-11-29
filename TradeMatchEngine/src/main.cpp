#include "MatchingEngine.hpp"
#include "Trader.hpp"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <vector>
#include <memory>
#include <string>

void printHeader(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void printStep(const std::string& description) {
    std::cout << "\n>>> " << description << std::endl;
}

void waitForUser() {
    std::cout << "\nPress Enter to continue..." << std::endl;
    std::cin.get();
}

void demonstrateBasicTrading(MatchingEngine& engine, 
                           const std::vector<std::shared_ptr<Trader>>& traders) {
    printHeader("BASIC TRADING DEMONSTRATION");
    
    printStep("Setting up initial positions");
    
    // Give some traders initial positions
    traders[0]->onOrderFilled("AAPL", 100.0, 145.0, true);  // Alice has 100 AAPL
    traders[1]->onOrderFilled("GOOGL", 50.0, 1950.0, true); // Bob has 50 GOOGL
    
    std::cout << "Initial positions created:" << std::endl;
    std::cout << "- Alice: 100 AAPL shares at avg $145" << std::endl;
    std::cout << "- Bob: 50 GOOGL shares at avg $1950" << std::endl;
    
    waitForUser();
    
    printStep("Alice places a sell order for AAPL");
    int aliceOrder = engine.submitOrder(1, "AAPL", 50.0, 150.0, OrderSide::SELL);
    std::cout << "Order ID " << aliceOrder << ": Sell 50 AAPL at $150.00" << std::endl;
    
    auto aaplBook = engine.getOrderBook("AAPL");
    aaplBook->printOrderBook();
    
    waitForUser();
    
    printStep("Charlie places a buy order for AAPL at a lower price");
    int charlieOrder1 = engine.submitOrder(3, "AAPL", 30.0, 148.0, OrderSide::BUY);
    std::cout << "Order ID " << charlieOrder1 << ": Buy 30 AAPL at $148.00" << std::endl;
    
    aaplBook->printOrderBook();
    
    waitForUser();
    
    printStep("Diana places a buy order that matches Alice's sell order");
    int dianaOrder = engine.submitOrder(4, "AAPL", 25.0, 150.0, OrderSide::BUY);
    std::cout << "Order ID " << dianaOrder << ": Buy 25 AAPL at $150.00" << std::endl;
    
    aaplBook->printOrderBook();
    
    waitForUser();
    
    printStep("Charlie raises his bid to match remaining shares");
    int charlieOrder2 = engine.submitOrder(3, "AAPL", 25.0, 150.0, OrderSide::BUY);
    std::cout << "Order ID " << charlieOrder2 << ": Buy 25 AAPL at $150.00" << std::endl;
    
    aaplBook->printOrderBook();
}

void demonstrateMarketDepth(MatchingEngine& engine,
                          const std::vector<std::shared_ptr<Trader>>& traders) {
    printHeader("MARKET DEPTH DEMONSTRATION");
    
    printStep("Creating a deep order book for MSFT");
    
    // Give traders some MSFT shares
    for (size_t i = 0; i < traders.size(); ++i) {
        if (i % 2 == 0) {  // Even indices get shares
            traders[i]->onOrderFilled("MSFT", 200.0, 290.0, true);
        }
    }
    
    // Create multiple price levels
    std::vector<int> orderIds;
    
    // Sell side (asks)
    orderIds.push_back(engine.submitOrder(2, "MSFT", 50.0, 305.0, OrderSide::SELL));
    orderIds.push_back(engine.submitOrder(4, "MSFT", 30.0, 302.0, OrderSide::SELL));
    orderIds.push_back(engine.submitOrder(6, "MSFT", 40.0, 300.0, OrderSide::SELL));
    
    // Buy side (bids)
    orderIds.push_back(engine.submitOrder(1, "MSFT", 35.0, 295.0, OrderSide::BUY));
    orderIds.push_back(engine.submitOrder(3, "MSFT", 45.0, 292.0, OrderSide::BUY));
    orderIds.push_back(engine.submitOrder(5, "MSFT", 25.0, 290.0, OrderSide::BUY));
    
    auto msftBook = engine.getOrderBook("MSFT");
    msftBook->printOrderBook();
    
    waitForUser();
    
    printStep("A large market order sweeps through multiple price levels");
    int marketSweep = engine.submitOrder(7, "MSFT", 100.0, 310.0, OrderSide::BUY);
    std::cout << "Order ID " << marketSweep << ": Buy 100 MSFT at up to $310.00" << std::endl;
    
    msftBook->printOrderBook();
    
    waitForUser();
    
    printStep("Market maker adds liquidity on both sides");
    int mmBid = engine.submitOrder(8, "MSFT", 50.0, 298.0, OrderSide::BUY);
    int mmAsk = engine.submitOrder(8, "MSFT", 50.0, 302.0, OrderSide::SELL);
    
    std::cout << "Market maker orders:" << std::endl;
    std::cout << "- Bid: Order ID " << mmBid << " for 50 MSFT at $298.00" << std::endl;
    std::cout << "- Ask: Order ID " << mmAsk << " for 50 MSFT at $302.00" << std::endl;
    
    msftBook->printOrderBook();
}

void demonstrateOrderManagement(MatchingEngine& engine,
                              const std::vector<std::shared_ptr<Trader>>& traders) {
    printHeader("ORDER MANAGEMENT DEMONSTRATION");
    
    printStep("Placing multiple orders and then canceling some");
    
    // Give Alice more shares
    traders[0]->onOrderFilled("TSLA", 150.0, 800.0, true);
    
    // Place several orders
    int order1 = engine.submitOrder(1, "TSLA", 25.0, 850.0, OrderSide::SELL);
    int order2 = engine.submitOrder(1, "TSLA", 30.0, 855.0, OrderSide::SELL);
    int order3 = engine.submitOrder(2, "TSLA", 20.0, 845.0, OrderSide::BUY);
    int order4 = engine.submitOrder(3, "TSLA", 15.0, 840.0, OrderSide::BUY);
    
    std::cout << "Placed orders:" << std::endl;
    std::cout << "- Order " << order1 << ": Sell 25 TSLA at $850" << std::endl;
    std::cout << "- Order " << order2 << ": Sell 30 TSLA at $855" << std::endl;
    std::cout << "- Order " << order3 << ": Buy 20 TSLA at $845" << std::endl;
    std::cout << "- Order " << order4 << ": Buy 15 TSLA at $840" << std::endl;
    
    auto tslaBook = engine.getOrderBook("TSLA");
    tslaBook->printOrderBook();
    
    waitForUser();
    
    printStep("Canceling Order " + std::to_string(order2));
    bool cancelled = engine.cancelOrder(order2);
    std::cout << "Cancel result: " << (cancelled ? "SUCCESS" : "FAILED") << std::endl;
    
    tslaBook->printOrderBook();
    
    waitForUser();
    
    printStep("Submitting order that matches existing bid");
    int matchingOrder = engine.submitOrder(1, "TSLA", 20.0, 845.0, OrderSide::SELL);
    std::cout << "Order ID " << matchingOrder << ": Sell 20 TSLA at $845.00" << std::endl;
    
    tslaBook->printOrderBook();
}

void demonstratePortfolioTracking(const std::vector<std::shared_ptr<Trader>>& traders) {
    printHeader("PORTFOLIO TRACKING DEMONSTRATION");
    
    printStep("Showing portfolio values after all trading activity");
    
    for (const auto& trader : traders) {
        trader->printPortfolio();
    }
}

void runPerformanceDemo(MatchingEngine& engine) {
    printHeader("PERFORMANCE DEMONSTRATION");
    
    printStep("Submitting 1000 orders as fast as possible");
    
    // Create a high-performance trader
    auto perfTrader = std::make_shared<Trader>(99, "PerfTrader", 10000000.0);
    
    // Give the trader massive positions
    perfTrader->onOrderFilled("PERF", 100000.0, 100.0, true);
    
    engine.registerTrader(perfTrader);
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    int successfulOrders = 0;
    for (int i = 0; i < 1000; ++i) {
        try {
            double price = 100.0 + (i % 20) * 0.1;
            OrderSide side = (i % 2 == 0) ? OrderSide::BUY : OrderSide::SELL;
            engine.submitOrder(99, "PERF", 10.0, price, side);
            successfulOrders++;
        } catch (...) {
            // Some may fail
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    std::cout << "Performance Results:" << std::endl;
    std::cout << "- Submitted " << successfulOrders << " orders" << std::endl;
    std::cout << "- Time taken: " << duration.count() << " microseconds" << std::endl;
    std::cout << "- Rate: " << std::fixed << std::setprecision(0) 
              << (successfulOrders * 1000000.0) / duration.count() 
              << " orders per second" << std::endl;
    
    auto perfBook = engine.getOrderBook("PERF");
    std::cout << "- Trades generated: " << perfBook->getTradeCount() << std::endl;
    std::cout << "- Total volume: " << perfBook->getTotalVolume() << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << std::fixed << std::setprecision(2);
    
    // Check for benchmark flag
    bool runBenchmark = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--benchmark") {
            runBenchmark = true;
            break;
        }
    }
    
    if (runBenchmark) {
        MatchingEngine engine;
        runPerformanceDemo(engine);
        return 0;
    }
    
    printHeader("TRADE MATCHING ENGINE DEMONSTRATION");
    
    std::cout << "Welcome to the Trade Matching Engine Demo!" << std::endl;
    std::cout << "This demonstration will show various features of the matching engine:" << std::endl;
    std::cout << "1. Basic order placement and matching" << std::endl;
    std::cout << "2. Market depth and price levels" << std::endl;
    std::cout << "3. Order management (cancellation)" << std::endl;
    std::cout << "4. Portfolio tracking" << std::endl;
    std::cout << "5. Performance testing" << std::endl;
    
    waitForUser();
    
    // Create matching engine
    MatchingEngine engine;
    
    // Create traders
    std::vector<std::shared_ptr<Trader>> traders = {
        std::make_shared<Trader>(1, "Alice", 100000.0),
        std::make_shared<Trader>(2, "Bob", 150000.0),
        std::make_shared<Trader>(3, "Charlie", 200000.0),
        std::make_shared<Trader>(4, "Diana", 175000.0),
        std::make_shared<Trader>(5, "Eve", 125000.0),
        std::make_shared<Trader>(6, "Frank", 300000.0),
        std::make_shared<Trader>(7, "Grace", 250000.0),
        std::make_shared<Trader>(8, "Henry", 400000.0)
    };
    
    // Register all traders
    for (const auto& trader : traders) {
        engine.registerTrader(trader);
    }
    
    try {
        // Run demonstrations
        demonstrateBasicTrading(engine, traders);
        waitForUser();
        
        demonstrateMarketDepth(engine, traders);
        waitForUser();
        
        demonstrateOrderManagement(engine, traders);
        waitForUser();
        
        demonstratePortfolioTracking(traders);
        waitForUser();
        
        runPerformanceDemo(engine);
        
        printHeader("FINAL MARKET SUMMARY");
        engine.printMarketSummary();
        
        std::cout << "\nEngine Statistics:" << std::endl;
        std::cout << "- Total trades: " << engine.getTotalTradeCount() << std::endl;
        std::cout << "- Total volume: " << engine.getTotalVolume() << std::endl;
        
        printHeader("DEMONSTRATION COMPLETE");
        std::cout << "Thank you for trying the Trade Matching Engine!" << std::endl;
        std::cout << "Key features demonstrated:" << std::endl;
        std::cout << "✓ Price-time priority matching" << std::endl;
        std::cout << "✓ Multiple symbol support" << std::endl;
        std::cout << "✓ Order book depth management" << std::endl;
        std::cout << "✓ Portfolio tracking and P&L" << std::endl;
        std::cout << "✓ Order cancellation" << std::endl;
        std::cout << "✓ High-performance order processing" << std::endl;
        std::cout << "✓ Real-time market data" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error during demonstration: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
