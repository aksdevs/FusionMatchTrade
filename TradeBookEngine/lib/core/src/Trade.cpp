#include "../include/TradeBookEngine/Core/Trade.hpp"
#include "../include/TradeBookEngine/Core/Utils.hpp"

using namespace TradeBookEngine::Core::Models;
using namespace TradeBookEngine::Core::Enums;
using namespace TradeBookEngine::Core::Utils;

Trade::Trade(const std::string& tradeId,
             AssetClass assetClass,
             const std::string& instrumentId,
             const std::string& counterparty,
             double notional,
             const std::string& currency,
             TradeSide side,
             const std::chrono::system_clock::time_point& tradeDate,
             const std::chrono::system_clock::time_point& settlementDate,
             const std::string& createdBy)
    : m_tradeId(tradeId)
    , m_assetClass(assetClass)
    , m_instrumentId(instrumentId)
    , m_counterparty(counterparty)
    , m_notional(notional)
    , m_currency(currency)
    , m_side(side)
    , m_tradeDate(tradeDate)
    , m_settlementDate(settlementDate)
    , m_createdBy(createdBy)
    , m_createdAt(std::chrono::system_clock::now())
    , m_status(TradeStatus::Pending) {
}