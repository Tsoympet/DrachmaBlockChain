#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <functional>
#include <leveldb/db.h>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../messages/crosschain_msg.cpp"
#include "../validation/proof_validator.h"
#include "../../../layer1-core/crypto/schnorr.h"

namespace crosschain {

struct BridgeLock {
    std::string id;
    std::string chain;
    std::string txid;
    std::string destination;
    uint64_t amount{0};
    std::array<uint8_t, 32> secretHash{};
    std::vector<uint8_t> secret;
    uint64_t timeoutHeight{0};
    bool inbound{false};
    bool claimed{false};
    bool refunded{false};
};

struct ChainConfig {
    std::array<uint8_t, 32> genesisHash{};
    std::string rpcEndpoint;
    bool watchOnly{false};
};

class BridgeManager {
public:
    explicit BridgeManager(const std::string& dbPath);

    bool RegisterChain(const std::string& name, const ChainConfig& config);

    BridgeLock InitiateOutboundLock(const std::string& chain,
                                    const std::string& txid,
                                    const std::string& destination,
                                    uint64_t amount,
                                    const std::array<uint8_t, 32>& secretHash,
                                    uint64_t timeoutHeight,
                                    const std::array<uint8_t, 32>& coordinatorPrivKey,
                                    std::vector<uint8_t>& signatureOut);

    bool DetectInboundLock(const std::string& chain,
                           const std::vector<HeaderProof>& proofs,
                           const BridgeLock& observedLock);

    bool Claim(const std::string& lockId,
               const std::vector<uint8_t>& secret,
               uint64_t currentHeight,
               std::vector<uint8_t>& signatureOut);

    bool Refund(const std::string& lockId, uint64_t currentHeight);

    std::optional<BridgeLock> GetLock(const std::string& lockId) const;

    std::vector<CrossChainMessage> PendingFor(const std::string& chain) const;

    void RelayMessage(const std::string& destChain, const CrossChainMessage& msg);

    std::array<uint8_t, 64> SignMessage(const std::array<uint8_t, 32>& privKey,
                                        const std::vector<uint8_t>& message) const;

private:
    std::string SerializeLock(const BridgeLock& lock) const;
    BridgeLock DeserializeLock(const std::string& raw) const;
    std::string NewLockId();
    void PersistLock(const BridgeLock& lock);

    std::unique_ptr<leveldb::DB> m_db;
    std::unordered_map<std::string, ChainConfig> m_chains;
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, std::vector<CrossChainMessage>> m_messages;
};

} // namespace crosschain

