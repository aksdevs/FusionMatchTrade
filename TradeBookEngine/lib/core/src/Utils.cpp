#include "../include/TradeBookEngine/Core/Utils.hpp"
#include "../include/TradeBookEngine/Core/Events/TradeBookedEvent.hpp"
#include <sstream>
#include <iomanip>
#include <random>
#include <set>

using namespace TradeBookEngine::Core::Utils;
using namespace TradeBookEngine::Core::Events;

// IdGenerator implementation
std::string IdGenerator::GenerateTradeId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(100000, 999999);
    
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    
    return "TRD-" + std::to_string(timestamp) + "-" + std::to_string(dis(gen));
}

std::string IdGenerator::GenerateCorrelationId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(100000, 999999);
    
    return "CORR-" + std::to_string(dis(gen));
}

// DateTimeUtils implementation
std::string DateTimeUtils::ToString(const std::chrono::system_clock::time_point& timePoint) {
    auto time_t = std::chrono::system_clock::to_time_t(timePoint);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

std::chrono::system_clock::time_point DateTimeUtils::FromString(const std::string& timeStr) {
    std::tm tm = {};
    std::stringstream ss(timeStr);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::chrono::system_clock::time_point DateTimeUtils::AddBusinessDays(
    const std::chrono::system_clock::time_point& startDate, 
    int businessDays) {
    
    auto current = startDate;
    int daysAdded = 0;
    
    while (daysAdded < businessDays) {
        current += std::chrono::hours(24);
        
        auto time_t = std::chrono::system_clock::to_time_t(current);
        auto tm = *std::localtime(&time_t);
        
        // Skip weekends (Saturday = 6, Sunday = 0)
        if (tm.tm_wday != 0 && tm.tm_wday != 6) {
            daysAdded++;
        }
    }
    
    return current;
}

// ValidationUtils implementation
bool ValidationUtils::IsValidCurrency(const std::string& currency) {
    static std::set<std::string> validCurrencies = {
        "USD", "EUR", "GBP", "JPY", "CHF", "CAD", "AUD", "NZD", "SEK", "NOK", "DKK"
    };
    
    return validCurrencies.find(currency) != validCurrencies.end();
}

bool ValidationUtils::IsValidNotional(double notional) {
    return notional > 0 && notional < 1e15; // Reasonable upper bound
}

bool ValidationUtils::IsValidCounterparty(const std::string& counterparty) {
    return !counterparty.empty() && counterparty.length() <= 100;
}

bool ValidationUtils::IsValidInstrumentId(const std::string& instrumentId) {
    return !instrumentId.empty() && instrumentId.length() <= 50;
}

// TradeBookedEvent implementation
TradeBookedEvent::TradeBookedEvent(std::shared_ptr<Models::Trade> trade, const std::string& correlationId)
    : m_trade(trade)
    , m_timestamp(std::chrono::system_clock::now())
    , m_eventId(IdGenerator::GenerateCorrelationId())
    , m_correlationId(correlationId.empty() ? trade->GetCorrelationId() : correlationId) {
}