#include "MatchingEngine.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>

MatchingEngine::MatchingEngine() : nextOrderId_(1) {}

void MatchingEngine::registerTrader(std::shared_ptr<Trader> trader) {
    traders_[trader->getTraderId()] = trader;
}

std::shared_ptr<Trader> MatchingEngine::getTrader(int traderId) const {
    auto it = traders_.find(traderId);
    return (it != traders_.end()) ? it->second : nullptr;
}

int MatchingEngine::submitOrder(int traderId, const std::string& symbol, double quantity,
                              double price, OrderSide side, OrderType type) {
    // Validate trader exists
    auto trader = getTrader(traderId);
    if (!trader) {
        throw std::invalid_argument("Trader not found");
    }
    
    // Pre-trade validation
    if (side == OrderSide::BUY) {
        double requiredCash = quantity * price;
        if (!trader->hasSufficientCash(requiredCash)) {
            throw std::runtime_error("Insufficient cash for buy order");
        }
    } else {
        if (!trader->hasSufficientShares(symbol, quantity)) {
            throw std::runtime_error("Insufficient shares for sell order");
        }
    }
    
    // Create order
    int orderId = nextOrderId_++;
    auto order = std::make_shared<Order>(orderId, traderId, symbol, quantity, price, side, type);
    orders_[orderId] = order;
    
    // Get or create order book for symbol
    auto orderBook = getOrCreateOrderBook(symbol);
    
    // Store trades before adding order
    size_t tradesBefore = orderBook->getTrades().size();
    
    // Add order to order book (this may execute trades)
    orderBook->addOrder(order);
    
    // Get new trades
    const auto& allTrades = orderBook->getTrades();
    std::vector<Trade> newTrades(allTrades.begin() + tradesBefore, allTrades.end());
    
    // Process trade notifications
    processTradeNotifications(newTrades);
    
    return orderId;
}

bool MatchingEngine::cancelOrder(int orderId) {
    auto orderIt = orders_.find(orderId);
    if (orderIt == orders_.end()) {
        return false;
    }
    
    auto order = orderIt->second;
    auto orderBook = getOrderBook(order->getSymbol());
    
    if (orderBook) {
        bool cancelled = orderBook->cancelOrder(orderId);
        if (cancelled) {
            orders_.erase(orderIt);
        }
        return cancelled;
    }
    
    return false;
}

std::shared_ptr<Order> MatchingEngine::getOrder(int orderId) const {
    auto it = orders_.find(orderId);
    return (it != orders_.end()) ? it->second : nullptr;
}

std::shared_ptr<OrderBook> MatchingEngine::getOrderBook(const std::string& symbol) {
    auto it = orderBooks_.find(symbol);
    return (it != orderBooks_.end()) ? it->second : nullptr;
}

std::shared_ptr<OrderBook> MatchingEngine::getOrCreateOrderBook(const std::string& symbol) {
    auto it = orderBooks_.find(symbol);
    if (it == orderBooks_.end()) {
        auto orderBook = std::make_shared<OrderBook>(symbol);
        orderBooks_[symbol] = orderBook;
        return orderBook;
    }
    return it->second;
}

double MatchingEngine::getLastPrice(const std::string& symbol) const {
    auto it = orderBooks_.find(symbol);
    return (it != orderBooks_.end()) ? it->second->getLastTradePrice() : 0.0;
}

double MatchingEngine::getBestBid(const std::string& symbol) const {
    auto it = orderBooks_.find(symbol);
    return (it != orderBooks_.end()) ? it->second->getBestBidPrice() : 0.0;
}

double MatchingEngine::getBestAsk(const std::string& symbol) const {
    auto it = orderBooks_.find(symbol);
    return (it != orderBooks_.end()) ? it->second->getBestAskPrice() : 0.0;
}

void MatchingEngine::processTradeNotifications(const std::vector<Trade>& trades) {
    for (const auto& trade : trades) {
        // Notify buyer
        auto buyer = getTrader(trade.buyTraderId);
        if (buyer) {
            buyer->onOrderFilled(trade.symbol, trade.quantity, trade.price, true);
        }
        
        // Notify seller
        auto seller = getTrader(trade.sellTraderId);
        if (seller) {
            seller->onOrderFilled(trade.symbol, trade.quantity, trade.price, false);
        }
        
        std::cout << "TRADE: " << trade.symbol 
                  << " | Qty: " << trade.quantity 
                  << " | Price: $" << std::fixed << std::setprecision(2) << trade.price
                  << " | Buyer: " << trade.buyTraderId 
                  << " | Seller: " << trade.sellTraderId << std::endl;
    }
}

void MatchingEngine::printMarketSummary() const {
    std::cout << "\n=== MARKET SUMMARY ===" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    
    std::cout << std::setw(10) << "Symbol"
              << std::setw(12) << "Last Price"
              << std::setw(12) << "Best Bid"
              << std::setw(12) << "Best Ask"
              << std::setw(10) << "Spread"
              << std::setw(12) << "Volume"
              << std::setw(10) << "Trades" << std::endl;
    std::cout << std::string(78, '-') << std::endl;
    
    for (const auto& [symbol, orderBook] : orderBooks_) {
        double lastPrice = orderBook->getLastTradePrice();
        double bestBid = orderBook->getBestBidPrice();
        double bestAsk = orderBook->getBestAskPrice();
        double spread = orderBook->getSpread();
        double volume = orderBook->getTotalVolume();
        size_t tradeCount = orderBook->getTradeCount();
        
        std::cout << std::setw(10) << symbol
                  << std::setw(12) << (lastPrice > 0 ? std::to_string(lastPrice) : "N/A")
                  << std::setw(12) << (bestBid > 0 ? std::to_string(bestBid) : "N/A")
                  << std::setw(12) << (bestAsk > 0 ? std::to_string(bestAsk) : "N/A")
                  << std::setw(10) << (spread > 0 ? std::to_string(spread) : "N/A")
                  << std::setw(12) << volume
                  << std::setw(10) << tradeCount << std::endl;
    }
    std::cout << "=====================\n" << std::endl;
}

void MatchingEngine::printAllOrderBooks() const {
    for (const auto& [symbol, orderBook] : orderBooks_) {
        orderBook->printOrderBook();
    }
}

std::vector<Trade> MatchingEngine::getAllTrades() const {
    std::vector<Trade> allTrades;
    
    for (const auto& [symbol, orderBook] : orderBooks_) {
        const auto& bookTrades = orderBook->getTrades();
        allTrades.insert(allTrades.end(), bookTrades.begin(), bookTrades.end());
    }
    
    return allTrades;
}

size_t MatchingEngine::getTotalTradeCount() const {
    size_t totalTrades = 0;
    for (const auto& [symbol, orderBook] : orderBooks_) {
        totalTrades += orderBook->getTradeCount();
    }
    return totalTrades;
}

double MatchingEngine::getTotalVolume() const {
    double totalVolume = 0.0;
    for (const auto& [symbol, orderBook] : orderBooks_) {
        totalVolume += orderBook->getTotalVolume();
    }
    return totalVolume;
}
