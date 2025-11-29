#include <iostream>
#include <memory>
#include "MatchingEngine.hpp"
#include "Trader.hpp"
#include "Order.hpp"

int main()
{
    MatchingEngine engine;

    auto trader1 = std::make_shared<Trader>(1, "Alice", 100000.0);
    auto trader2 = std::make_shared<Trader>(2, "Bob", 100000.0);

    engine.registerTrader(trader1);
    engine.registerTrader(trader2);

    // Give trader2 initial shares so they can sell
    trader2->onOrderFilled("SYM", 10, 10.0, true);

    // Submit a sell order from trader2
    int sellOrderId = engine.submitOrder(2, "SYM", 10, 10.0, OrderSide::SELL);

    // Submit a buy order from trader1 that should match
    int buyOrderId = engine.submitOrder(1, "SYM", 10, 12.0, OrderSide::BUY);

    auto totalTrades = engine.getTotalTradeCount();
    if (totalTrades < 1)
    {
        std::cerr << "Expected at least one trade, got " << totalTrades << std::endl;
        return 1;
    }

    std::cout << "matchengine test passed (trades=" << totalTrades << ")" << std::endl;
    return 0;
}
