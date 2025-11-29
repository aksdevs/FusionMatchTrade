#pragma once

#include <memory>
#include "../Events/TradeBookedEvent.hpp"

namespace TradeBookEngine {
namespace Core {
namespace Interfaces {

    class IEventPublisher {
    public:
        virtual ~IEventPublisher() = default;
        
        virtual void Publish(const Events::TradeBookedEvent& event) = 0;
    };

} // namespace Interfaces
} // namespace Core
} // namespace TradeBookEngine