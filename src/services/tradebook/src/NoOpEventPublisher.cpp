#include "../include/TradeBookEngine/Core/Interfaces/IEventPublisher.hpp"
#include <iostream>

using namespace TradeBookEngine::Core::Interfaces;
using namespace TradeBookEngine::Core::Events;

class NoOpEventPublisher : public IEventPublisher
{
public:
    void Publish(const TradeBookedEvent &event) override
    {
        // No-op implementation - just log to console
        std::cout << "Event published: Trade " << event.GetTrade()->GetTradeId()
                  << " booked at " << event.GetEventId() << std::endl;
    }
};

// Factory function
extern "C"
{
    IEventPublisher *CreateNoOpEventPublisher()
    {
        return new NoOpEventPublisher();
    }

    void DestroyNoOpEventPublisher(IEventPublisher *publisher)
    {
        delete publisher;
    }
}
