#include "relayer.h"

#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <chrono>
#include <iostream>
#include <vector>

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

namespace crosschain {

Relayer::Relayer(BridgeManager& bridge, net::P2PNode& p2p, boost::asio::io_context& io)
    : m_bridge(bridge), m_p2p(p2p), m_io(io)
{
}

Relayer::~Relayer()
{
    Stop();
}

void Relayer::AddWatchedChain(const std::string& name, const ChainConfig& cfg)
{
    m_bridge.RegisterChain(name, cfg);
    std::lock_guard<std::mutex> g(m_mutex);
    m_endpoints[name] = cfg.rpcEndpoint;
}

void Relayer::Start()
{
    m_stop = false;
    m_thread = std::thread([this] { Loop(); });
}

void Relayer::Stop()
{
    m_stop = true;
    if (m_thread.joinable()) m_thread.join();
}

void Relayer::Loop()
{
    while (!m_stop) {
        try {
            Tick();
        } catch (const std::exception& e) {
            std::cerr << "Relayer tick error: " << e.what() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void Relayer::Tick()
{
    std::unordered_map<std::string, std::string> endpoints;
    {
        std::lock_guard<std::mutex> g(m_mutex);
        endpoints = m_endpoints;
    }

    for (const auto& [chain, endpoint] : endpoints) {
        auto proof = FetchLatestProof(endpoint);
        if (!proof) continue;

        BridgeLock lock{};
        lock.chain = chain;
        lock.txid = "observed";
        lock.destination = "drachma";
        lock.amount = 1;
        lock.secretHash = {};
        lock.timeoutHeight = 0;

        if (m_bridge.DetectInboundLock(chain, {*proof}, lock)) {
            std::vector<uint8_t> header(proof->header.begin(), proof->header.end());
            net::Message msg{"xchain", std::move(header)};
            m_p2p.Broadcast(msg);
            ++m_metrics.detected;
        }
    }
}

std::optional<HeaderProof> Relayer::FetchLatestProof(const std::string& endpoint)
{
    if (endpoint.empty()) return std::nullopt;
    std::string host;
    std::string port = "80";
    std::string target = "/";

    auto pos = endpoint.find("://");
    std::string trimmed = pos == std::string::npos ? endpoint : endpoint.substr(pos + 3);
    auto slash = trimmed.find('/');
    host = slash == std::string::npos ? trimmed : trimmed.substr(0, slash);
    if (slash != std::string::npos) target = trimmed.substr(slash);
    auto colon = host.find(':');
    if (colon != std::string::npos) {
        port = host.substr(colon + 1);
        host = host.substr(0, colon);
    }

    tcp::resolver resolver(m_io);
    tcp::socket socket(m_io);
    auto results = resolver.resolve(host, port);
    boost::asio::connect(socket, results.begin(), results.end());

    http::request<http::string_body> req{http::verb::get, target, 11};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, "drachma-relayer");

    http::write(socket, req);
    boost::beast::flat_buffer buffer;
    http::response<http::string_body> res;
    http::read(socket, buffer, res);

    socket.shutdown(tcp::socket::shutdown_both);

    HeaderProof proof{};
    if (res.result() != http::status::ok || res.body().size() < proof.header.size()) {
        return std::nullopt;
    }
    std::copy(res.body().begin(), res.body().begin() + proof.header.size(), proof.header.begin());
    proof.height = static_cast<uint32_t>(res.body().size());
    return proof;
}

RelayerMetrics Relayer::Metrics() const
{
    return m_metrics;
}

} // namespace crosschain

