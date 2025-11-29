#include "../include/TradeBookEngine/Core/TradeService.hpp"
#include "../include/TradeBookEngine/Core/Utils.hpp"
#include "../include/TradeBookEngine/Core/Events/TradeBookedEvent.hpp"
#include <stdexcept>
#include <algorithm>

using namespace TradeBookEngine::Core::Services;
using namespace TradeBookEngine::Core::Models;
using namespace TradeBookEngine::Core::Interfaces;
using namespace TradeBookEngine::Core::Validators;
using namespace TradeBookEngine::Core::Utils;
using namespace TradeBookEngine::Core::Events;

TradeService::TradeService(std::shared_ptr<ITradeRepository> repository,
                           std::shared_ptr<IEventPublisher> eventPublisher)
    : m_repository(repository), m_eventPublisher(eventPublisher)
{
}

void TradeService::AddValidator(std::shared_ptr<IAssetValidator> validator)
{
    m_validators.push_back(validator);
}

std::shared_ptr<Trade> TradeService::BookTrade(const TradeDto &tradeDto)
{
    // Check for duplicate idempotency key
    if (!tradeDto.IdempotencyKey.empty())
    {
        auto existingTrade = m_repository->GetByIdempotencyKey(tradeDto.IdempotencyKey);
        if (existingTrade)
        {
            return existingTrade; // Return existing trade for idempotency
        }
    }

    // Validate the trade
    ValidateTrade(tradeDto);

    // Convert DTO to Trade model
    auto trade = ConvertToTrade(tradeDto);

    // Set status to booked
    trade->SetStatus(Enums::TradeStatus::Booked);

    // Save to repository
    m_repository->Save(trade);

    // Publish event
    TradeBookedEvent event(trade, tradeDto.CorrelationId);
    m_eventPublisher->Publish(event);

    return trade;
}

std::shared_ptr<Trade> TradeService::GetTrade(const std::string &tradeId)
{
    return m_repository->GetById(tradeId);
}

std::vector<std::shared_ptr<Trade>> TradeService::GetTradesByCounterparty(const std::string &counterparty)
{
    return m_repository->GetByCounterparty(counterparty);
}

std::vector<std::shared_ptr<Trade>> TradeService::GetAllTrades()
{
    return m_repository->GetAll();
}

void TradeService::ValidateTrade(const TradeDto &tradeDto)
{
    // Basic validation
    if (tradeDto.InstrumentId.empty())
    {
        throw std::invalid_argument("InstrumentId cannot be empty");
    }
    if (tradeDto.Counterparty.empty())
    {
        throw std::invalid_argument("Counterparty cannot be empty");
    }
    if (tradeDto.Notional <= 0)
    {
        throw std::invalid_argument("Notional must be positive");
    }
    if (tradeDto.Currency.empty())
    {
        throw std::invalid_argument("Currency cannot be empty");
    }

    // Asset-specific validation
    auto validator = std::find_if(m_validators.begin(), m_validators.end(),
                                  [&tradeDto](const std::shared_ptr<IAssetValidator> &v)
                                  {
                                      return v->GetSupportedAssetClass() == tradeDto.AssetClass;
                                  });

    if (validator != m_validators.end())
    {
        if (!(*validator)->IsValid(tradeDto))
        {
            auto errors = (*validator)->GetValidationErrors(tradeDto);
            std::string errorMsg = "Validation failed: ";
            for (const auto &error : errors)
            {
                errorMsg += error + "; ";
            }
            throw std::invalid_argument(errorMsg);
        }
    }
}

std::shared_ptr<Trade> TradeService::ConvertToTrade(const TradeDto &tradeDto)
{
    std::string tradeId = tradeDto.TradeId.empty() ? IdGenerator::GenerateTradeId() : tradeDto.TradeId;

    auto trade = std::make_shared<Trade>(
        tradeId,
        tradeDto.AssetClass,
        tradeDto.InstrumentId,
        tradeDto.Counterparty,
        tradeDto.Notional,
        tradeDto.Currency,
        tradeDto.Side,
        tradeDto.TradeDate,
        tradeDto.SettlementDate,
        tradeDto.CreatedBy);

    if (!tradeDto.IdempotencyKey.empty())
    {
        trade->SetIdempotencyKey(tradeDto.IdempotencyKey);
    }
    if (!tradeDto.CorrelationId.empty())
    {
        trade->SetCorrelationId(tradeDto.CorrelationId);
    }

    // Copy additional data
    for (const auto &pair : tradeDto.Additional)
    {
        trade->AddAdditionalData(pair.first, pair.second);
    }

    return trade;
}

// C-style factory functions for console app
extern "C"
{
    TradeService *CreateTradeService(ITradeRepository *repository, IEventPublisher *eventPublisher)
    {
        return new TradeService(
            std::shared_ptr<ITradeRepository>(repository, [](ITradeRepository *) {}),
            std::shared_ptr<IEventPublisher>(eventPublisher, [](IEventPublisher *) {}));
    }

    void DestroyTradeService(TradeService *service)
    {
        delete service;
    }

    Trade *BookTradeC(TradeService *service, const TradeDto *tradeDto)
    {
        try
        {
            auto trade = service->BookTrade(*tradeDto);
            return trade.get();
        }
        catch (...)
        {
            return nullptr;
        }
    }
}
