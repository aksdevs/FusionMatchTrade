#pragma once

#include <string>
#include <chrono>
#include <random>

namespace TradeBookEngine {
namespace Core {
namespace Utils {

    class IdGenerator {
    public:
        static std::string GenerateTradeId();
        static std::string GenerateCorrelationId();
    };

    class DateTimeUtils {
    public:
        static std::string ToString(const std::chrono::system_clock::time_point& timePoint);
        static std::chrono::system_clock::time_point FromString(const std::string& timeStr);
        static std::chrono::system_clock::time_point AddBusinessDays(
            const std::chrono::system_clock::time_point& startDate, 
            int businessDays
        );
    };

    class ValidationUtils {
    public:
        static bool IsValidCurrency(const std::string& currency);
        static bool IsValidNotional(double notional);
        static bool IsValidCounterparty(const std::string& counterparty);
        static bool IsValidInstrumentId(const std::string& instrumentId);
    };

} // namespace Utils
} // namespace Core
} // namespace TradeBookEngine