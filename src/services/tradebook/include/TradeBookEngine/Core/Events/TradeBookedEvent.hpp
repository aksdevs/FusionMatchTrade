#pragma once

#include <memory>
#include <string>
#include <chrono>
#include "../Trade.hpp"

namespace TradeBookEngine
{
    namespace Core
    {
        namespace Events
        {

            class TradeBookedEvent
            {
            private:
                std::shared_ptr<Models::Trade> m_trade;
                std::chrono::system_clock::time_point m_timestamp;
                std::string m_eventId;
                std::string m_correlationId;

            public:
                TradeBookedEvent(std::shared_ptr<Models::Trade> trade,
                                 const std::string &correlationId = "");

                // Getters
                std::shared_ptr<Models::Trade> GetTrade() const { return m_trade; }
                const std::chrono::system_clock::time_point &GetTimestamp() const { return m_timestamp; }
                const std::string &GetEventId() const { return m_eventId; }
                const std::string &GetCorrelationId() const { return m_correlationId; }
            };

        } // namespace Events
    } // namespace Core
} // namespace TradeBookEngine
