#pragma once
#include <string>
#include <chrono>
#include <memory>

enum class OrderSide {
    BUY,
    SELL
};

enum class OrderType {
    MARKET,
    LIMIT,
    STOP,
    STOP_LIMIT
};

enum class OrderStatus {
    PENDING,
    PARTIALLY_FILLED,
    FILLED,
    CANCELLED,
    REJECTED
};

class Order {
public:
    Order(int orderId, int traderId, const std::string& symbol, 
          double quantity, double price, OrderSide side, OrderType type = OrderType::LIMIT);
    
    // Getters
    int getOrderId() const { return orderId_; }
    int getTraderId() const { return traderId_; }
    const std::string& getSymbol() const { return symbol_; }
    double getQuantity() const { return quantity_; }
    double getPrice() const { return price_; }
    OrderSide getSide() const { return side_; }
    OrderType getType() const { return type_; }
    OrderStatus getStatus() const { return status_; }
    double getFilledQuantity() const { return filledQuantity_; }
    double getRemainingQuantity() const { return quantity_ - filledQuantity_; }
    std::chrono::steady_clock::time_point getTimestamp() const { return timestamp_; }
    
    // Setters
    void setStatus(OrderStatus status) { status_ = status; }
    void addFill(double quantity);
    
    // Utility methods
    bool isComplete() const { return filledQuantity_ >= quantity_; }
    bool isBuy() const { return side_ == OrderSide::BUY; }
    bool isSell() const { return side_ == OrderSide::SELL; }
    
    // For priority queue comparison (price-time priority)
    bool operator<(const Order& other) const;
    
private:
    int orderId_;
    int traderId_;
    std::string symbol_;
    double quantity_;
    double price_;
    OrderSide side_;
    OrderType type_;
    OrderStatus status_;
    double filledQuantity_;
    std::chrono::steady_clock::time_point timestamp_;
};
