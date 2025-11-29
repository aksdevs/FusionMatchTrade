#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <chrono>

#include "TradeBookEngine/Core/TradeService.hpp"
#include "TradeBookEngine/Core/Validators/IAssetValidator.hpp"
#include "TradeBookEngine/Core/Interfaces/ITradeRepository.hpp"
#include "TradeBookEngine/Core/Interfaces/IEventPublisher.hpp"
#include "TradeBookEngine/Core/TradeDto.hpp"
#include "TradeBookEngine/Core/Enums.hpp"

using namespace TradeBookEngine::Core;
using namespace TradeBookEngine::Core::Services;
using namespace TradeBookEngine::Core::Validators;
using namespace TradeBookEngine::Core::Interfaces;
using namespace TradeBookEngine::Core::Models;
using namespace TradeBookEngine::Core::Enums;

// Declare factory functions from core library (C linkage)
extern "C" {
    ITradeRepository* CreateInMemoryTradeRepository();
    void DestroyInMemoryTradeRepository(ITradeRepository*);
    IEventPublisher* CreateNoOpEventPublisher();
    void DestroyNoOpEventPublisher(IEventPublisher*);
    IAssetValidator* CreateEquityValidator();
    IAssetValidator* CreateBondValidator();
    void DestroyValidator(IAssetValidator*);
}

struct TestContext {
    std::shared_ptr<ITradeRepository> repo;
    std::shared_ptr<IEventPublisher> publisher;
    std::shared_ptr<IAssetValidator> equityValidator;

    TradeService* serviceRaw{nullptr};
    std::unique_ptr<TradeService> service;

    TestContext() {
        repo = std::shared_ptr<ITradeRepository>(
            CreateInMemoryTradeRepository(),
            [](ITradeRepository* p){ DestroyInMemoryTradeRepository(p); }
        );
        publisher = std::shared_ptr<IEventPublisher>(
            CreateNoOpEventPublisher(),
            [](IEventPublisher* p){ DestroyNoOpEventPublisher(p); }
        );
        equityValidator = std::shared_ptr<IAssetValidator>(
            CreateEquityValidator(),
            [](IAssetValidator* v){ DestroyValidator(v); }
        );
        service = std::make_unique<TradeService>(repo, publisher);
        service->AddValidator(equityValidator);
    }
};

static int failures = 0;

#define CHECK(cond, msg) \
    do { \
        if(!(cond)) { \
            ++failures; \
            std::cerr << "[FAIL] " << msg << "\n"; \
        } else { \
            std::cout << "[PASS] " << msg << "\n"; \
        } \
    } while(0)

TradeDto MakeValidEquityDto() {
    TradeDto dto;
    dto.AssetClass = AssetClass::Equity;
    dto.InstrumentId = "AAPL";
    dto.Counterparty = "Counterparty1";
    dto.Notional = 100000.0;
    dto.Currency = "USD";
    dto.Side = TradeSide::Buy;
    dto.TradeDate = std::chrono::system_clock::now();
    dto.SettlementDate = dto.TradeDate;
    dto.CreatedBy = "tester";
    dto.Additional["Exchange"] = "NASDAQ";
    dto.IdempotencyKey = "idem-123";
    dto.CorrelationId = "corr-123";
    return dto;
}

void test_book_trade_happy_path() {
    TestContext ctx;
    auto dto = MakeValidEquityDto();
    auto trade = ctx.service->BookTrade(dto);

    CHECK(trade != nullptr, "BookTrade returns a trade");
    CHECK(trade->GetStatus() == TradeStatus::Booked, "Trade status is Booked");
    CHECK(trade->GetAssetClass() == AssetClass::Equity, "AssetClass is Equity");
    CHECK(trade->GetNotional() == dto.Notional, "Notional copied");
    CHECK(trade->GetCurrency() == dto.Currency, "Currency copied");
    CHECK(trade->GetInstrumentId() == dto.InstrumentId, "InstrumentId copied");
    CHECK(trade->GetCounterparty() == dto.Counterparty, "Counterparty copied");
}

void test_idempotency_returns_existing() {
    TestContext ctx;
    auto dto = MakeValidEquityDto();
    auto t1 = ctx.service->BookTrade(dto);
    auto t2 = ctx.service->BookTrade(dto); // same idempotency key

    CHECK(t1 != nullptr && t2 != nullptr, "Trades created for idempotent submissions");
    CHECK(t1->GetTradeId() == t2->GetTradeId(), "Second book returns existing trade");
}

void test_validation_failure() {
    TestContext ctx;
    auto dto = MakeValidEquityDto();
    dto.Notional = -10.0; // invalid

    bool threw = false;
    try {
        (void)ctx.service->BookTrade(dto);
    } catch (const std::invalid_argument&) {
        threw = true;
    } catch (...) {
        threw = false;
    }
    CHECK(threw, "Invalid notional throws invalid_argument");
}

int main() {
    std::cout << "Running TradeBookEngine tests...\n";
    test_book_trade_happy_path();
    test_idempotency_returns_existing();
    test_validation_failure();

    if (failures == 0) {
        std::cout << "All tests passed.\n";
        return 0;
    } else {
        std::cerr << failures << " test(s) failed." << std::endl;
        return 1;
    }
}
