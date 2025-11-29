#pragma once

#include <string>
#include <chrono>
#include <unordered_map>
#include "Enums.hpp"

namespace TradeBookEngine {
namespace Core {
namespace Models {

    struct TradeDto {
        // Basic trade information
        std::string TradeId;
        Enums::AssetClass AssetClass;
        std::string InstrumentId;
        std::string Counterparty;
        double Notional;
        std::string Currency;
        Enums::TradeSide Side;
        
        // Dates
        std::chrono::system_clock::time_point TradeDate;
        std::chrono::system_clock::time_point SettlementDate;
        
        // Additional metadata
        std::unordered_map<std::string, std::string> Additional;
        
        // Processing metadata
        std::string IdempotencyKey;
        std::string CorrelationId;
        std::string CreatedBy;
        std::chrono::system_clock::time_point CreatedAt;
        
        // Status
        Enums::TradeStatus Status;
        
        TradeDto()
            : AssetClass(Enums::AssetClass::Equity)
            , Notional(0.0)
            , Side(Enums::TradeSide::Buy)
            , TradeDate(std::chrono::system_clock::now())
            , CreatedAt(std::chrono::system_clock::now())
            , Status(Enums::TradeStatus::Pending) {
        }
    };

} // namespace Models
} // namespace Core
} // namespace TradeBookEngine