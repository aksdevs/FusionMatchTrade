#include "OrderBook.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <limits>

OrderBook::OrderBook(const std::string& symbol) : symbol_(symbol) {}

void OrderBook::addOrder(std::shared_ptr<Order> order) {
    if (order->getSymbol() != symbol_) {
        throw std::invalid_argument("Order symbol does not match order book symbol");
    }
    
    // Add to order map for quick lookup
    orderMap_[order->getOrderId()] = order;
    
    // Try to match the order
    auto newTrades = matchOrder(order);
    trades_.insert(trades_.end(), newTrades.begin(), newTrades.end());
    
    // If order is not completely filled, add to appropriate queue
    if (!order->isComplete()) {
        if (order->isBuy()) {
            bids_.push(order);
        } else {
            asks_.push(order);
        }
    }
    
    // Clean up completed orders
    removeCompletedOrders();
}

bool OrderBook::cancelOrder(int orderId) {
    auto it = orderMap_.find(orderId);
    if (it != orderMap_.end()) {
        it->second->setStatus(OrderStatus::CANCELLED);
        orderMap_.erase(it);
        removeCompletedOrders();
        return true;
    }
    return false;
}

std::shared_ptr<Order> OrderBook::getOrder(int orderId) const {
    auto it = orderMap_.find(orderId);
    return (it != orderMap_.end()) ? it->second : nullptr;
}

double OrderBook::getBestBidPrice() const {
    if (bids_.empty()) {
        return 0.0;
    }
    
    // Make a copy to peek at the top without modifying the queue
    auto bidsCopy = bids_;
    while (!bidsCopy.empty()) {
        auto topOrder = bidsCopy.top();
        bidsCopy.pop();
        
        if (!topOrder->isComplete() && topOrder->getStatus() != OrderStatus::CANCELLED) {
            return topOrder->getPrice();
        }
    }
    return 0.0;
}

double OrderBook::getBestAskPrice() const {
    if (asks_.empty()) {
        return 0.0;
    }
    
    // Make a copy to peek at the top without modifying the queue
    auto asksCopy = asks_;
    while (!asksCopy.empty()) {
        auto topOrder = asksCopy.top();
        asksCopy.pop();
        
        if (!topOrder->isComplete() && topOrder->getStatus() != OrderStatus::CANCELLED) {
            return topOrder->getPrice();
        }
    }
    return 0.0;
}

double OrderBook::getSpread() const {
    double bestBid = getBestBidPrice();
    double bestAsk = getBestAskPrice();
    
    if (bestBid > 0.0 && bestAsk > 0.0) {
        return bestAsk - bestBid;
    }
    return 0.0;
}

double OrderBook::getLastTradePrice() const {
    return trades_.empty() ? 0.0 : trades_.back().price;
}

double OrderBook::getTotalVolume() const {
    double totalVolume = 0.0;
    for (const auto& trade : trades_) {
        totalVolume += trade.quantity;
    }
    return totalVolume;
}

std::vector<Trade> OrderBook::matchOrder(std::shared_ptr<Order> newOrder) {
    std::vector<Trade> trades;
    
    if (newOrder->isBuy()) {
        // Match buy order against sell orders
        while (!asks_.empty() && !newOrder->isComplete()) {
            auto bestAsk = asks_.top();
            
            // Skip completed or cancelled orders
            if (bestAsk->isComplete() || bestAsk->getStatus() == OrderStatus::CANCELLED) {
                asks_.pop();
                continue;
            }
            
            // Check if prices cross
            if (newOrder->getPrice() >= bestAsk->getPrice()) {
                asks_.pop();
                
                // Calculate trade quantity
                double tradeQuantity = std::min(newOrder->getRemainingQuantity(), 
                                               bestAsk->getRemainingQuantity());
                double tradePrice = bestAsk->getPrice(); // Price improvement for incoming order
                
                // Execute trade
                newOrder->addFill(tradeQuantity);
                bestAsk->addFill(tradeQuantity);
                
                // Record trade
                trades.emplace_back(
                    newOrder->getOrderId(), bestAsk->getOrderId(),
                    newOrder->getTraderId(), bestAsk->getTraderId(),
                    symbol_, tradeQuantity, tradePrice
                );
                
                // If sell order still has remaining quantity, put it back
                if (!bestAsk->isComplete()) {
                    asks_.push(bestAsk);
                }
            } else {
                break; // No more matches possible
            }
        }
    } else {
        // Match sell order against buy orders
        while (!bids_.empty() && !newOrder->isComplete()) {
            auto bestBid = bids_.top();
            
            // Skip completed or cancelled orders
            if (bestBid->isComplete() || bestBid->getStatus() == OrderStatus::CANCELLED) {
                bids_.pop();
                continue;
            }
            
            // Check if prices cross
            if (newOrder->getPrice() <= bestBid->getPrice()) {
                bids_.pop();
                
                // Calculate trade quantity
                double tradeQuantity = std::min(newOrder->getRemainingQuantity(), 
                                               bestBid->getRemainingQuantity());
                double tradePrice = bestBid->getPrice(); // Price improvement for incoming order
                
                // Execute trade
                newOrder->addFill(tradeQuantity);
                bestBid->addFill(tradeQuantity);
                
                // Record trade
                trades.emplace_back(
                    bestBid->getOrderId(), newOrder->getOrderId(),
                    bestBid->getTraderId(), newOrder->getTraderId(),
                    symbol_, tradeQuantity, tradePrice
                );
                
                // If buy order still has remaining quantity, put it back
                if (!bestBid->isComplete()) {
                    bids_.push(bestBid);
                }
            } else {
                break; // No more matches possible
            }
        }
    }
    
    return trades;
}

void OrderBook::removeCompletedOrders() {
    // Remove completed orders from bids
    std::priority_queue<std::shared_ptr<Order>> newBids;
    while (!bids_.empty()) {
        auto order = bids_.top();
        bids_.pop();
        
        if (!order->isComplete() && order->getStatus() != OrderStatus::CANCELLED) {
            newBids.push(order);
        } else {
            orderMap_.erase(order->getOrderId());
        }
    }
    bids_ = std::move(newBids);
    
    // Remove completed orders from asks
    std::priority_queue<std::shared_ptr<Order>> newAsks;
    while (!asks_.empty()) {
        auto order = asks_.top();
        asks_.pop();
        
        if (!order->isComplete() && order->getStatus() != OrderStatus::CANCELLED) {
            newAsks.push(order);
        } else {
            orderMap_.erase(order->getOrderId());
        }
    }
    asks_ = std::move(newAsks);
}

void OrderBook::printOrderBook() const {
    std::cout << "\n=== Order Book for " << symbol_ << " ===" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    
    // Print asks (sells) in descending price order
    std::cout << "\nAsks (Sells):" << std::endl;
    auto asksCopy = asks_;
    std::vector<std::shared_ptr<Order>> askList;
    
    while (!asksCopy.empty()) {
        auto order = asksCopy.top();
        asksCopy.pop();
        if (!order->isComplete() && order->getStatus() != OrderStatus::CANCELLED) {
            askList.push_back(order);
        }
    }
    
    std::sort(askList.begin(), askList.end(), 
              [](const auto& a, const auto& b) { return a->getPrice() > b->getPrice(); });
    
    for (const auto& order : askList) {
        std::cout << "  $" << order->getPrice() << " x " << order->getRemainingQuantity() << std::endl;
    }
    
    std::cout << "\n--- Spread: $" << getSpread() << " ---" << std::endl;
    
    // Print bids (buys) in descending price order
    std::cout << "\nBids (Buys):" << std::endl;
    auto bidsCopy = bids_;
    std::vector<std::shared_ptr<Order>> bidList;
    
    while (!bidsCopy.empty()) {
        auto order = bidsCopy.top();
        bidsCopy.pop();
        if (!order->isComplete() && order->getStatus() != OrderStatus::CANCELLED) {
            bidList.push_back(order);
        }
    }
    
    for (const auto& order : bidList) {
        std::cout << "  $" << order->getPrice() << " x " << order->getRemainingQuantity() << std::endl;
    }
    
    std::cout << "\nLast Trade: $" << getLastTradePrice() << std::endl;
    std::cout << "Total Volume: " << getTotalVolume() << std::endl;
    std::cout << "===========================\n" << std::endl;
}
