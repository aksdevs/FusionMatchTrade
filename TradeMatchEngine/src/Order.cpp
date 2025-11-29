#include "Order.hpp"
#include <stdexcept>

Order::Order(int orderId, int traderId, const std::string& symbol,
             double quantity, double price, OrderSide side, OrderType type)
    : orderId_(orderId), traderId_(traderId), symbol_(symbol),
      quantity_(quantity), price_(price), side_(side), type_(type),
      status_(OrderStatus::PENDING), filledQuantity_(0.0),
      timestamp_(std::chrono::steady_clock::now()) {
    
    if (quantity <= 0) {
        throw std::invalid_argument("Quantity must be positive");
    }
    
    if (type == OrderType::LIMIT && price <= 0) {
        throw std::invalid_argument("Price must be positive for limit orders");
    }
}

void Order::addFill(double quantity) {
    if (quantity <= 0) {
        throw std::invalid_argument("Fill quantity must be positive");
    }
    
    if (filledQuantity_ + quantity > quantity_) {
        throw std::invalid_argument("Fill quantity exceeds remaining order quantity");
    }
    
    filledQuantity_ += quantity;
    
    if (isComplete()) {
        status_ = OrderStatus::FILLED;
    } else {
        status_ = OrderStatus::PARTIALLY_FILLED;
    }
}

bool Order::operator<(const Order& other) const {
    // For buy orders: higher price has higher priority
    // For sell orders: lower price has higher priority
    // If prices are equal, earlier timestamp has higher priority
    
    if (side_ == OrderSide::BUY) {
        if (price_ != other.price_) {
            return price_ < other.price_;  // Higher price wins for buy orders
        }
    } else {  // SELL
        if (price_ != other.price_) {
            return price_ > other.price_;  // Lower price wins for sell orders
        }
    }
    
    // If prices are equal, earlier timestamp wins
    return timestamp_ > other.timestamp_;
}
