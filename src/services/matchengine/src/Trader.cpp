#include "Trader.hpp"
#include <iostream>
#include <iomanip>

Trader::Trader(int traderId, const std::string &name, double initialCash)
    : traderId_(traderId), name_(name), cash_(initialCash) {}

double Trader::getPortfolioValue() const
{
    double totalValue = cash_;

    for (const auto &[symbol, position] : positions_)
    {
        totalValue += position.quantity * position.averagePrice;
    }

    return totalValue;
}

void Trader::addCash(double amount)
{
    if (amount < 0 && (-amount) > cash_)
    {
        throw std::invalid_argument("Insufficient cash for withdrawal");
    }
    cash_ += amount;
}

bool Trader::hasSufficientShares(const std::string &symbol, double quantity) const
{
    auto it = positions_.find(symbol);
    return (it != positions_.end() && it->second.quantity >= quantity);
}

void Trader::onOrderFilled(const std::string &symbol, double quantity, double price, bool isBuy)
{
    if (isBuy)
    {
        double cost = quantity * price;
        if (cost > cash_)
        {
            throw std::runtime_error("Insufficient cash for purchase");
        }
        cash_ -= cost;
        updatePositionOnTrade(symbol, quantity, price);
    }
    else
    {
        if (!hasSufficientShares(symbol, quantity))
        {
            throw std::runtime_error("Insufficient shares for sale");
        }
        cash_ += quantity * price;
        updatePositionOnTrade(symbol, -quantity, price);
    }
}

void Trader::updatePosition(const std::string &symbol, double marketPrice)
{
    auto it = positions_.find(symbol);
    if (it != positions_.end())
    {
        Position &position = it->second;
        position.unrealizedPnL = position.quantity * (marketPrice - position.averagePrice);
    }
}

void Trader::updatePositionOnTrade(const std::string &symbol, double quantity, double price)
{
    auto it = positions_.find(symbol);

    if (it == positions_.end())
    {
        if (quantity > 0)
        {
            positions_[symbol] = Position(symbol, quantity, price);
        }
    }
    else
    {
        Position &position = it->second;

        if ((position.quantity > 0 && quantity > 0) || (position.quantity < 0 && quantity < 0))
        {
            double totalCost = (position.quantity * position.averagePrice) + (quantity * price);
            double totalQuantity = position.quantity + quantity;

            if (totalQuantity != 0)
            {
                position.averagePrice = totalCost / totalQuantity;
            }
            position.quantity = totalQuantity;
        }
        else
        {
            double remainingQuantity = position.quantity + quantity;

            if (std::abs(remainingQuantity) < 1e-9)
            {
                positions_.erase(it);
            }
            else if ((position.quantity > 0 && remainingQuantity > 0) ||
                     (position.quantity < 0 && remainingQuantity < 0))
            {
                position.quantity = remainingQuantity;
            }
            else
            {
                position.quantity = remainingQuantity;
                position.averagePrice = price;
            }
        }

        if (it != positions_.end() && std::abs(it->second.quantity) < 1e-9)
        {
            positions_.erase(it);
        }
    }
}

void Trader::printPortfolio() const
{
    std::cout << "\n=== Portfolio for " << name_ << " (ID: " << traderId_ << ") ===" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Cash: $" << cash_ << std::endl;

    if (!positions_.empty())
    {
        std::cout << "\nPositions:" << std::endl;
        std::cout << std::setw(10) << "Symbol"
                  << std::setw(12) << "Quantity"
                  << std::setw(15) << "Avg Price"
                  << std::setw(15) << "Market Value"
                  << std::setw(15) << "Unrealized P&L" << std::endl;
        std::cout << std::string(67, '-') << std::endl;

        for (const auto &[symbol, position] : positions_)
        {
            double marketValue = position.quantity * position.averagePrice;
            std::cout << std::setw(10) << symbol
                      << std::setw(12) << position.quantity
                      << std::setw(15) << position.averagePrice
                      << std::setw(15) << marketValue
                      << std::setw(15) << position.unrealizedPnL << std::endl;
        }
    }

    std::cout << "\nTotal Portfolio Value: $" << getPortfolioValue() << std::endl;
    std::cout << "================================\n"
              << std::endl;
}

double Trader::getTotalPnL() const
{
    double totalPnL = 0.0;
    for (const auto &[symbol, position] : positions_)
    {
        totalPnL += position.unrealizedPnL;
    }
    return totalPnL;
}
