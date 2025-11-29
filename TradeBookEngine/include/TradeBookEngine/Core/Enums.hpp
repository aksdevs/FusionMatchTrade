#pragma once

namespace TradeBookEngine {
namespace Core {
namespace Enums {

    enum class AssetClass {
        Equity,
        Bond,
        Derivative,
        Commodity,
        Currency
    };

    enum class TradeStatus {
        Pending,
        Booked,
        Settled,
        Cancelled,
        Failed
    };

    enum class TradeSide {
        Buy,
        Sell
    };

} // namespace Enums
} // namespace Core
} // namespace TradeBookEngine