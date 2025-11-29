#include <iostream>
#include <memory>
#include <chrono>
#include <vector>

#include "TradeBookEngine/Core/TradeDto.hpp"
#include "TradeBookEngine/Core/Enums.hpp"
#include "TradeBookEngine/Core/Utils.hpp"
#include "TradeBookEngine/Core/TradeService.hpp"
#include "TradeBookEngine/Core/Interfaces/ITradeRepository.hpp"
#include "TradeBookEngine/Core/Interfaces/IEventPublisher.hpp"
#include "TradeBookEngine/Core/Validators/IAssetValidator.hpp"

using namespace TradeBookEngine::Core::Models;
using namespace TradeBookEngine::Core::Enums;
using namespace TradeBookEngine::Core::Utils;
using namespace TradeBookEngine::Core::Interfaces;
using namespace TradeBookEngine::Core::Services;
using namespace TradeBookEngine::Core::Validators;

// Factory functions implemented in the core static library (extern "C")
extern "C" ITradeRepository* CreateInMemoryTradeRepository();
extern "C" IEventPublisher* CreateNoOpEventPublisher();
extern "C" IAssetValidator* CreateEquityValidator();
extern "C" IAssetValidator* CreateBondValidator();

int main() {
    std::cout << "=== Trade Booking Engine Console Application ===" << std::endl;
    
    try {
        // Create dependencies
        auto repo = std::shared_ptr<ITradeRepository>(CreateInMemoryTradeRepository());
        auto publisher = std::shared_ptr<IEventPublisher>(CreateNoOpEventPublisher());
        auto equityValidator = std::shared_ptr<IAssetValidator>(CreateEquityValidator());
        auto bondValidator = std::shared_ptr<IAssetValidator>(CreateBondValidator());

        // Create trade service
        auto tradeService = std::make_shared<TradeService>(repo, publisher);
        tradeService->AddValidator(equityValidator);
        tradeService->AddValidator(bondValidator);

        std::cout << "\nBooking sample trades..." << std::endl;

        // Create and book equity trade
        TradeDto equityDto;
        equityDto.AssetClass = AssetClass::Equity;
        equityDto.InstrumentId = "MSFT";
        equityDto.Counterparty = "Goldman Sachs";
        equityDto.Notional = 1000000.0;
        equityDto.Currency = "USD";
        equityDto.Side = TradeSide::Buy;
        equityDto.SettlementDate = std::chrono::system_clock::now() + std::chrono::hours(48);
        equityDto.Additional["Exchange"] = "NASDAQ";
        equityDto.IdempotencyKey = "EQ-ORD-1001";
        equityDto.CorrelationId = "corr-equity-1";
        equityDto.CreatedBy = "ConsoleApp";

        auto equityTrade = tradeService->BookTrade(equityDto);
        std::cout << "✓ Equity trade booked: " << equityTrade->GetTradeId() 
                  << " for " << equityTrade->GetNotional() << " " << equityTrade->GetCurrency() << std::endl;

        // Create and book bond trade
        TradeDto bondDto;
        bondDto.AssetClass = AssetClass::Bond;
        bondDto.InstrumentId = "US10Y";
        bondDto.Counterparty = "JP Morgan";
        bondDto.Notional = 5000000.0;
        bondDto.Currency = "USD";
        bondDto.Side = TradeSide::Sell;
        bondDto.SettlementDate = std::chrono::system_clock::now() + std::chrono::hours(72);
        bondDto.Additional["MaturityDate"] = "2034-01-15";
        bondDto.Additional["CreditRating"] = "AAA";
        bondDto.IdempotencyKey = "BOND-ORD-2001";
        bondDto.CorrelationId = "corr-bond-1";
        bondDto.CreatedBy = "ConsoleApp";

        auto bondTrade = tradeService->BookTrade(bondDto);
        std::cout << "✓ Bond trade booked: " << bondTrade->GetTradeId() 
                  << " for " << bondTrade->GetNotional() << " " << bondTrade->GetCurrency() << std::endl;

        // Test idempotency - try to book the same equity trade again
        std::cout << "\nTesting idempotency..." << std::endl;
        auto duplicateTrade = tradeService->BookTrade(equityDto);
        std::cout << "✓ Duplicate trade returned existing: " << duplicateTrade->GetTradeId() 
                  << " (same as original)" << std::endl;

        // Display all trades
        std::cout << "\nAll trades in repository:" << std::endl;
        auto allTrades = tradeService->GetAllTrades();
        for (const auto& trade : allTrades) {
            std::cout << "  - " << trade->GetTradeId() << ": "
                      << (trade->GetAssetClass() == AssetClass::Equity ? "Equity" : "Bond")
                      << " " << trade->GetInstrumentId()
                      << " (" << trade->GetCounterparty() << ")" << std::endl;
        }

        // Test validation error
        std::cout << "\nTesting validation (should fail)..." << std::endl;
        TradeDto invalidDto;
        invalidDto.AssetClass = AssetClass::Equity;
        invalidDto.InstrumentId = "AAPL";
        invalidDto.Counterparty = "";  // Invalid - empty counterparty
        invalidDto.Notional = -1000.0; // Invalid - negative notional
        invalidDto.Currency = "USD";
        
        try {
            tradeService->BookTrade(invalidDto);
            std::cout << "✗ Should have failed validation" << std::endl;
        } catch (const std::exception& ex) {
            std::cout << "✓ Validation correctly failed: " << ex.what() << std::endl;
        }

        std::cout << "\n=== Trade Booking Engine Demo Complete ===" << std::endl;
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}