#pragma once

#include <vector>
#include <string>
#include "../TradeDto.hpp"
#include "../Enums.hpp"

namespace TradeBookEngine {
namespace Core {
namespace Validators {

    class IAssetValidator {
    public:
        virtual ~IAssetValidator() = default;
        
        virtual bool IsValid(const Models::TradeDto& tradeDto) const = 0;
        virtual std::vector<std::string> GetValidationErrors(const Models::TradeDto& tradeDto) const = 0;
        virtual Enums::AssetClass GetSupportedAssetClass() const = 0;
    };

} // namespace Validators
} // namespace Core
} // namespace TradeBookEngine