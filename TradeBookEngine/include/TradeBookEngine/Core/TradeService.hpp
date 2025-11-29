#pragma once

#include <memory>
#include <vector>
#include "Trade.hpp"
#include "TradeDto.hpp"
#include "Interfaces/ITradeRepository.hpp"
#include "Interfaces/IEventPublisher.hpp"
#include "Validators/IAssetValidator.hpp"

namespace TradeBookEngine {
namespace Core {
namespace Services {

    class TradeService {
    private:
        std::shared_ptr<Interfaces::ITradeRepository> m_repository;
        std::shared_ptr<Interfaces::IEventPublisher> m_eventPublisher;
        std::vector<std::shared_ptr<Validators::IAssetValidator>> m_validators;

    public:
        TradeService(std::shared_ptr<Interfaces::ITradeRepository> repository,
                    std::shared_ptr<Interfaces::IEventPublisher> eventPublisher);

        void AddValidator(std::shared_ptr<Validators::IAssetValidator> validator);
        
        std::shared_ptr<Models::Trade> BookTrade(const Models::TradeDto& tradeDto);
        std::shared_ptr<Models::Trade> GetTrade(const std::string& tradeId);
        std::vector<std::shared_ptr<Models::Trade>> GetTradesByCounterparty(const std::string& counterparty);
        std::vector<std::shared_ptr<Models::Trade>> GetAllTrades();

    private:
        void ValidateTrade(const Models::TradeDto& tradeDto);
        std::shared_ptr<Models::Trade> ConvertToTrade(const Models::TradeDto& tradeDto);
    };

} // namespace Services
} // namespace Core
} // namespace TradeBookEngine