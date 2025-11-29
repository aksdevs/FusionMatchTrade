#pragma once
#include "Order.hpp"
#include <queue>
#include <map>
#include <vector>
#include <memory>

struct Trade
{
    int buyOrderId;
    int sellOrderId;
    int buyTraderId;
    int sellTraderId;
    std::string symbol;
    double quantity;
    double price;
    std::chrono::steady_clock::time_point timestamp;

    Trade(int buyOrderId, int sellOrderId, int buyTraderId, int sellTraderId,
          const std::string &symbol, double quantity, double price)
        : buyOrderId(buyOrderId), sellOrderId(sellOrderId),
          buyTraderId(buyTraderId), sellTraderId(sellTraderId),
          symbol(symbol), quantity(quantity), price(price),
          timestamp(std::chrono::steady_clock::now()) {}
};

class OrderBook
{
public:
    explicit OrderBook(const std::string &symbol);

    // Order management
    void addOrder(std::shared_ptr<Order> order);
    bool cancelOrder(int orderId);
    std::shared_ptr<Order> getOrder(int orderId) const;

    // Market data
    double getBestBidPrice() const;
    double getBestAskPrice() const;
    double getSpread() const;
    size_t getBidDepth() const { return bids_.size(); }
    size_t getAskDepth() const { return asks_.size(); }

    // Order book state
    const std::string &getSymbol() const { return symbol_; }
    const std::vector<Trade> &getTrades() const { return trades_; }

    // Statistics
    double getLastTradePrice() const;
    double getTotalVolume() const;
    size_t getTradeCount() const { return trades_.size(); }

    void printOrderBook() const;

private:
    std::string symbol_;

    // Priority queues for orders (best prices at top)
    // Use a comparator that dereferences the shared_ptr to compare Orders
    struct OrderPtrCompare
    {
        bool operator()(const std::shared_ptr<Order> &a, const std::shared_ptr<Order> &b) const
        {
            return *a < *b;
        }
    };

    std::priority_queue<std::shared_ptr<Order>, std::vector<std::shared_ptr<Order>>, OrderPtrCompare> bids_; // Buy orders (highest price first)
    std::priority_queue<std::shared_ptr<Order>, std::vector<std::shared_ptr<Order>>, OrderPtrCompare> asks_; // Sell orders (lowest price first)

    // Map for quick order lookup
    std::map<int, std::shared_ptr<Order>> orderMap_;

    // Trade history
    std::vector<Trade> trades_;

    // Helper methods
    std::vector<Trade> matchOrder(std::shared_ptr<Order> newOrder);
    void removeCompletedOrders();
};
