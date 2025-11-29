#pragma once
#include <string>
#include <map>
#include <vector>
#include <memory>

struct Position
{
    std::string symbol;
    double quantity;
    double averagePrice;
    double unrealizedPnL;

    Position(const std::string &sym = "", double qty = 0.0, double avgPrice = 0.0)
        : symbol(sym), quantity(qty), averagePrice(avgPrice), unrealizedPnL(0.0) {}
};

class Trader
{
public:
    Trader(int traderId, const std::string &name, double initialCash = 100000.0);

    // Getters
    int getTraderId() const { return traderId_; }
    const std::string &getName() const { return name_; }
    double getCash() const { return cash_; }
    double getPortfolioValue() const;
    const std::map<std::string, Position> &getPositions() const { return positions_; }

    // Portfolio management
    void addCash(double amount);
    bool hasSufficientCash(double amount) const { return cash_ >= amount; }
    bool hasSufficientShares(const std::string &symbol, double quantity) const;

    // Trade execution callbacks
    void onOrderFilled(const std::string &symbol, double quantity, double price, bool isBuy);
    void updatePosition(const std::string &symbol, double marketPrice);

    // Portfolio reporting
    void printPortfolio() const;
    double getTotalPnL() const;

private:
    int traderId_;
    std::string name_;
    double cash_;
    std::map<std::string, Position> positions_;

    void updatePositionOnTrade(const std::string &symbol, double quantity, double price);
};
