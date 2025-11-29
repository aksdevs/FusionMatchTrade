#include <iostream>
#include <memory>
#include "TradeBookEngine/Core/TradeService.hpp"
#include "TradeBookEngine/Core/TradeDto.hpp"
#include "TradeBookEngine/Core/Enums.hpp"

// Factory declarations for repo and publisher (defined in the service library)
extern "C" TradeBookEngine::Core::Interfaces::ITradeRepository *CreateInMemoryTradeRepository();
extern "C" void DestroyInMemoryTradeRepository(TradeBookEngine::Core::Interfaces::ITradeRepository *repo);
extern "C" TradeBookEngine::Core::Interfaces::IEventPublisher *CreateNoOpEventPublisher();
extern "C" void DestroyNoOpEventPublisher(TradeBookEngine::Core::Interfaces::IEventPublisher *pub);

int main()
{
    using namespace TradeBookEngine::Core;

    auto rawRepo = CreateInMemoryTradeRepository();
    auto rawPub = CreateNoOpEventPublisher();

    std::shared_ptr<Interfaces::ITradeRepository> repo(rawRepo, [](Interfaces::ITradeRepository *p)
                                                       { DestroyInMemoryTradeRepository(p); });
    std::shared_ptr<Interfaces::IEventPublisher> pub(rawPub, [](Interfaces::IEventPublisher *p)
                                                     { DestroyNoOpEventPublisher(p); });

    Services::TradeService service(repo, pub);

    Models::TradeDto dto;
    dto.InstrumentId = "TESTSYM";
    dto.Counterparty = "CP1";
    dto.Notional = 1000.0;
    dto.Currency = "USD";
    dto.Side = Enums::TradeSide::Buy;
    dto.CreatedBy = "test";

    auto trade = service.BookTrade(dto);
    if (!trade)
    {
        std::cerr << "BookTrade returned null" << std::endl;
        return 2;
    }

    if (trade->GetInstrumentId() != dto.InstrumentId)
    {
        std::cerr << "InstrumentId mismatch" << std::endl;
        return 3;
    }

    std::cout << "tradebook test passed" << std::endl;
    return 0;
}
