#include "../include/TradeBookEngine/Core/Validators/IAssetValidator.hpp"
#include "../include/TradeBookEngine/Core/Utils.hpp"
#include <vector>
#include <string>
#include <algorithm>

using namespace TradeBookEngine::Core::Validators;
using namespace TradeBookEngine::Core::Models;
using namespace TradeBookEngine::Core::Enums;
using namespace TradeBookEngine::Core::Utils;

class EquityValidator : public IAssetValidator {
public:
    bool IsValid(const TradeDto& tradeDto) const override {
        return GetValidationErrors(tradeDto).empty();
    }

    std::vector<std::string> GetValidationErrors(const TradeDto& tradeDto) const override {
        std::vector<std::string> errors;

        if (tradeDto.AssetClass != AssetClass::Equity) {
            errors.push_back("Invalid asset class for equity validator");
        }

        if (!ValidationUtils::IsValidInstrumentId(tradeDto.InstrumentId)) {
            errors.push_back("Invalid instrument ID for equity");
        }

        if (!ValidationUtils::IsValidNotional(tradeDto.Notional)) {
            errors.push_back("Invalid notional amount");
        }

        // Check for required additional fields for equity
        auto it = tradeDto.Additional.find("Exchange");
        if (it == tradeDto.Additional.end() || it->second.empty()) {
            errors.push_back("Exchange is required for equity trades");
        }

        return errors;
    }

    AssetClass GetSupportedAssetClass() const override {
        return AssetClass::Equity;
    }
};

class BondValidator : public IAssetValidator {
public:
    bool IsValid(const TradeDto& tradeDto) const override {
        return GetValidationErrors(tradeDto).empty();
    }

    std::vector<std::string> GetValidationErrors(const TradeDto& tradeDto) const override {
        std::vector<std::string> errors;

        if (tradeDto.AssetClass != AssetClass::Bond) {
            errors.push_back("Invalid asset class for bond validator");
        }

        if (!ValidationUtils::IsValidInstrumentId(tradeDto.InstrumentId)) {
            errors.push_back("Invalid instrument ID for bond");
        }

        if (!ValidationUtils::IsValidNotional(tradeDto.Notional)) {
            errors.push_back("Invalid notional amount");
        }

        // Check for required additional fields for bonds
        auto maturityIt = tradeDto.Additional.find("MaturityDate");
        if (maturityIt == tradeDto.Additional.end() || maturityIt->second.empty()) {
            errors.push_back("MaturityDate is required for bond trades");
        }

        auto ratingIt = tradeDto.Additional.find("CreditRating");
        if (ratingIt == tradeDto.Additional.end() || ratingIt->second.empty()) {
            errors.push_back("CreditRating is required for bond trades");
        }

        return errors;
    }

    AssetClass GetSupportedAssetClass() const override {
        return AssetClass::Bond;
    }
};

// Factory functions
extern "C" {
    IAssetValidator* CreateEquityValidator() {
        return new EquityValidator();
    }

    IAssetValidator* CreateBondValidator() {
        return new BondValidator();
    }

    void DestroyValidator(IAssetValidator* validator) {
        delete validator;
    }
}