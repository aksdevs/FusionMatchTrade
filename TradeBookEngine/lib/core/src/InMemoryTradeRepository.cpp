#include "../include/TradeBookEngine/Core/Interfaces/ITradeRepository.hpp"
#include <unordered_map>
#include <algorithm>
#include <mutex>

using namespace TradeBookEngine::Core::Interfaces;
using namespace TradeBookEngine::Core::Models;

class InMemoryTradeRepository : public ITradeRepository {
private:
    std::unordered_map<std::string, std::shared_ptr<Trade>> m_tradesById;
    std::unordered_map<std::string, std::shared_ptr<Trade>> m_tradesByIdempotencyKey;
    mutable std::mutex m_mutex;

public:
    void Save(std::shared_ptr<Trade> trade) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tradesById[trade->GetTradeId()] = trade;
        
        if (!trade->GetIdempotencyKey().empty()) {
            m_tradesByIdempotencyKey[trade->GetIdempotencyKey()] = trade;
        }
    }

    std::shared_ptr<Trade> GetById(const std::string& tradeId) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_tradesById.find(tradeId);
        return it != m_tradesById.end() ? it->second : nullptr;
    }

    std::shared_ptr<Trade> GetByIdempotencyKey(const std::string& idempotencyKey) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_tradesByIdempotencyKey.find(idempotencyKey);
        return it != m_tradesByIdempotencyKey.end() ? it->second : nullptr;
    }

    std::vector<std::shared_ptr<Trade>> GetByCounterparty(const std::string& counterparty) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<std::shared_ptr<Trade>> result;
        
        for (const auto& pair : m_tradesById) {
            if (pair.second->GetCounterparty() == counterparty) {
                result.push_back(pair.second);
            }
        }
        
        return result;
    }

    std::vector<std::shared_ptr<Trade>> GetAll() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<std::shared_ptr<Trade>> result;
        
        for (const auto& pair : m_tradesById) {
            result.push_back(pair.second);
        }
        
        return result;
    }

    bool Exists(const std::string& tradeId) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_tradesById.find(tradeId) != m_tradesById.end();
    }

    void Delete(const std::string& tradeId) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_tradesById.find(tradeId);
        if (it != m_tradesById.end()) {
            // Also remove from idempotency key map if exists
            const auto& idempotencyKey = it->second->GetIdempotencyKey();
            if (!idempotencyKey.empty()) {
                m_tradesByIdempotencyKey.erase(idempotencyKey);
            }
            m_tradesById.erase(it);
        }
    }
};

// Factory function
extern "C" {
    ITradeRepository* CreateInMemoryTradeRepository() {
        return new InMemoryTradeRepository();
    }

    void DestroyInMemoryTradeRepository(ITradeRepository* repository) {
        delete repository;
    }
}