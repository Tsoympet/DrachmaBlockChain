#pragma once

#include <atomic>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>

#include <boost/asio/io_context.hpp>

#include "../bridge/bridge_manager.h"
#include "../../net/p2p.h"

namespace crosschain {

struct RelayerMetrics {
    std::atomic<uint64_t> detected{0};

    RelayerMetrics() = default;
    RelayerMetrics(const RelayerMetrics& other)
        : detected(other.detected.load(std::memory_order_relaxed)) {}
    RelayerMetrics& operator=(const RelayerMetrics& other)
    {
        detected.store(other.detected.load(std::memory_order_relaxed), std::memory_order_relaxed);
        return *this;
    }
};

class Relayer {
public:
    Relayer(BridgeManager& bridge, net::P2PNode& p2p, boost::asio::io_context& io);
    ~Relayer();

    void AddWatchedChain(const std::string& name, const ChainConfig& cfg);
    void Start();
    void Stop();
    RelayerMetrics Metrics() const;

private:
    void Loop();
    void Tick();
    std::optional<HeaderProof> FetchLatestProof(const std::string& endpoint);

    BridgeManager& m_bridge;
    net::P2PNode& m_p2p;
    boost::asio::io_context& m_io;
    std::thread m_thread;
    std::atomic<bool> m_stop{false};
    std::unordered_map<std::string, std::string> m_endpoints;
    mutable std::mutex m_mutex;
    RelayerMetrics m_metrics;
};

} // namespace crosschain

