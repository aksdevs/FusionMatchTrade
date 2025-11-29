#pragma once
#include "OrderBook.hpp"
#include "Trader.hpp"
#include <map>
#include <memory>
#include <vector>

class MatchingEngine
{
public:
    MatchingEngine();
    ~MatchingEngine() = default;

    // Trader management
    void registerTrader(std::shared_ptr<Trader> trader);
    std::shared_ptr<Trader> getTrader(int traderId) const;

    // Order management
    int submitOrder(int traderId, const std::string &symbol, double quantity,
                    double price, OrderSide side, OrderType type = OrderType::LIMIT);
    bool cancelOrder(int orderId);
    std::shared_ptr<Order> getOrder(int orderId) const;

    // Market data
    std::shared_ptr<OrderBook> getOrderBook(const std::string &symbol);
    double getLastPrice(const std::string &symbol) const;
    double getBestBid(const std::string &symbol) const;
    double getBestAsk(const std::string &symbol) const;

    // Statistics and reporting
    void printMarketSummary() const;
    void printAllOrderBooks() const;
    std::vector<Trade> getAllTrades() const;

    // Engine state
    size_t getTotalTradeCount() const;
    double getTotalVolume() const;

private:
    int nextOrderId_;
    std::map<std::string, std::shared_ptr<OrderBook>> orderBooks_;
    std::map<int, std::shared_ptr<Trader>> traders_;
    std::map<int, std::shared_ptr<Order>> orders_;

    // Helper methods
    void processTradeNotifications(const std::vector<Trade> &trades);
    std::shared_ptr<OrderBook> getOrCreateOrderBook(const std::string &symbol);
};
