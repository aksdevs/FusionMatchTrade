#pragma once

#include <vector>
#include <memory>
#include "../Trade.hpp"

namespace TradeBookEngine
{
    namespace Core
    {
        namespace Interfaces
        {

            class ITradeRepository
            {
            public:
                virtual ~ITradeRepository() = default;

                virtual void Save(std::shared_ptr<Models::Trade> trade) = 0;
                virtual std::shared_ptr<Models::Trade> GetById(const std::string &tradeId) = 0;
                virtual std::shared_ptr<Models::Trade> GetByIdempotencyKey(const std::string &idempotencyKey) = 0;
                virtual std::vector<std::shared_ptr<Models::Trade>> GetByCounterparty(const std::string &counterparty) = 0;
                virtual std::vector<std::shared_ptr<Models::Trade>> GetAll() = 0;
                virtual bool Exists(const std::string &tradeId) = 0;
                virtual void Delete(const std::string &tradeId) = 0;
            };

        } // namespace Interfaces
    } // namespace Core
} // namespace TradeBookEngine
