#include "sidechain/bridge/peg_tx.h"

#include <gtest/gtest.h>

#include <array>
#include <string>
#include <vector>

#include <openssl/sha.h>

using sidechain::bridge::lock_tx;
using sidechain::bridge::peg_bridge;
using sidechain::bridge::spv_proof;

namespace {
std::array<uint8_t, 32> double_sha256(const std::vector<uint8_t>& data) {
    std::array<uint8_t, 32> first{};
    SHA256(data.data(), data.size(), first.data());

    std::array<uint8_t, 32> second{};
    SHA256(first.data(), first.size(), second.data());
    return second;
}

std::array<uint8_t, 32> compute_root_for_tx(const std::string& tx_id) {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i + 1 < tx_id.size(); i += 2) {
        bytes.push_back(static_cast<uint8_t>(std::stoul(tx_id.substr(i, 2), nullptr, 16)));
    }
    if (bytes.empty()) {
        bytes.assign(tx_id.begin(), tx_id.end());
    }
    return double_sha256(bytes);
}
}

TEST(PegBridge, FullPegCycle) {
    peg_bridge bridge{6, 10};

    const lock_tx lock{.tx_id = "a1b2c3", .from = "alice", .to = "carol", .amount = 100, .is_lock = true};

    spv_proof proof{};
    proof.tx_id = lock.tx_id;
    proof.merkle_root = compute_root_for_tx(lock.tx_id);
    proof.confirmations = 6;
    proof.mainnet_height = 100;
    proof.expiration_height = 120;

    auto minted = bridge.mint_wrapped(lock, proof, 105);
    ASSERT_TRUE(minted.success);
    EXPECT_EQ(minted.amount, lock.amount);
    EXPECT_EQ(bridge.balance_of(lock.to), 100);
    EXPECT_TRUE(bridge.is_processed(lock.tx_id));

    auto replay = bridge.mint_wrapped(lock, proof, 105);
    EXPECT_FALSE(replay.success);
    EXPECT_EQ(bridge.balance_of(lock.to), 100);

    auto burn = bridge.burn_wrapped(lock.to, 40);
    ASSERT_TRUE(burn.success);
    EXPECT_EQ(burn.amount, 40);
    EXPECT_EQ(bridge.balance_of(lock.to), 60);
    EXPECT_EQ(burn.burn.from, lock.to);
    EXPECT_FALSE(burn.burn.burn_id.empty());
}

TEST(PegBridge, ExpiredProofFails) {
    peg_bridge bridge{3, 5};
    const lock_tx lock{.tx_id = "deadbeef", .from = "bob", .to = "dave", .amount = 50, .is_lock = true};

    spv_proof proof{};
    proof.tx_id = lock.tx_id;
    proof.merkle_root = compute_root_for_tx(lock.tx_id);
    proof.confirmations = 3;
    proof.mainnet_height = 90;
    proof.expiration_height = 95;

    auto result = bridge.mint_wrapped(lock, proof, 200);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(bridge.balance_of(lock.to), 0);
}

