#pragma once

#include <string>
#include <chrono>
#include <unordered_map>
#include "Enums.hpp"

namespace TradeBookEngine {
namespace Core {
namespace Models {

    class Trade {
    private:
        std::string m_tradeId;
        Enums::AssetClass m_assetClass;
        std::string m_instrumentId;
        std::string m_counterparty;
        double m_notional;
        std::string m_currency;
        Enums::TradeSide m_side;
        std::chrono::system_clock::time_point m_tradeDate;
        std::chrono::system_clock::time_point m_settlementDate;
        std::unordered_map<std::string, std::string> m_additional;
        std::string m_idempotencyKey;
        std::string m_correlationId;
        std::string m_createdBy;
        std::chrono::system_clock::time_point m_createdAt;
        Enums::TradeStatus m_status;

    public:
        // Constructor
        Trade(const std::string& tradeId,
              Enums::AssetClass assetClass,
              const std::string& instrumentId,
              const std::string& counterparty,
              double notional,
              const std::string& currency,
              Enums::TradeSide side,
              const std::chrono::system_clock::time_point& tradeDate,
              const std::chrono::system_clock::time_point& settlementDate,
              const std::string& createdBy);

        // Getters
        const std::string& GetTradeId() const { return m_tradeId; }
        Enums::AssetClass GetAssetClass() const { return m_assetClass; }
        const std::string& GetInstrumentId() const { return m_instrumentId; }
        const std::string& GetCounterparty() const { return m_counterparty; }
        double GetNotional() const { return m_notional; }
        const std::string& GetCurrency() const { return m_currency; }
        Enums::TradeSide GetSide() const { return m_side; }
        const std::chrono::system_clock::time_point& GetTradeDate() const { return m_tradeDate; }
        const std::chrono::system_clock::time_point& GetSettlementDate() const { return m_settlementDate; }
        const std::unordered_map<std::string, std::string>& GetAdditional() const { return m_additional; }
        const std::string& GetIdempotencyKey() const { return m_idempotencyKey; }
        const std::string& GetCorrelationId() const { return m_correlationId; }
        const std::string& GetCreatedBy() const { return m_createdBy; }
        const std::chrono::system_clock::time_point& GetCreatedAt() const { return m_createdAt; }
        Enums::TradeStatus GetStatus() const { return m_status; }

        // Setters
        void SetStatus(Enums::TradeStatus status) { m_status = status; }
        void SetIdempotencyKey(const std::string& key) { m_idempotencyKey = key; }
        void SetCorrelationId(const std::string& id) { m_correlationId = id; }
        void AddAdditionalData(const std::string& key, const std::string& value) {
            m_additional[key] = value;
        }
    };

} // namespace Models
} // namespace Core
} // namespace TradeBookEngine