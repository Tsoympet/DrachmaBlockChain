#include <gtest/gtest.h>
#include <openssl/sha.h>
#include <algorithm>
#include <array>
#include <vector>
#include "../../layer2-services/crosschain/validation/proof_validator.h"

namespace {
std::array<uint8_t, 32> DoubleSha(const std::array<uint8_t, 80>& header)
{
    std::array<uint8_t, 32> hash{};
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, header.data(), header.size());
    SHA256_Final(hash.data(), &ctx);
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, hash.data(), hash.size());
    SHA256_Final(hash.data(), &ctx);
    return hash;
}

std::array<uint8_t, 80> BuildHeader(const std::array<uint8_t, 32>& prev_hash, uint32_t nonce)
{
    std::array<uint8_t, 80> header{};
    header[0] = 1;
    std::copy(prev_hash.begin(), prev_hash.end(), header.begin() + 4);
    for (size_t i = 0; i < 32; ++i) {
        header[36 + i] = static_cast<uint8_t>(i);
    }
    header[68] = 1;
    header[72] = 2;
    header[76] = static_cast<uint8_t>(nonce & 0xFF);
    header[77] = static_cast<uint8_t>((nonce >> 8) & 0xFF);
    header[78] = static_cast<uint8_t>((nonce >> 16) & 0xFF);
    header[79] = static_cast<uint8_t>((nonce >> 24) & 0xFF);
    return header;
}
} // namespace

TEST(ProofValidator, AcceptsLinkedHeaders)
{
    std::array<uint8_t, 32> zero{};
    auto header1 = BuildHeader(zero, 1);
    auto hash1 = DoubleSha(header1);
    auto header2 = BuildHeader(hash1, 2);

    crosschain::HeaderProof proof1{header1, 1};
    crosschain::HeaderProof proof2{header2, 2};
    std::vector<crosschain::HeaderProof> proofs{proof1, proof2};

    crosschain::ProofValidator validator;
    auto expected_tip = DoubleSha(header2);
    EXPECT_TRUE(validator.ValidateChain(proofs, expected_tip));
}

TEST(ProofValidator, RejectsWrongPrevHash)
{
    std::array<uint8_t, 32> zero{};
    auto header1 = BuildHeader(zero, 1);
    auto header2 = BuildHeader(zero, 2);

    crosschain::HeaderProof proof1{header1, 1};
    crosschain::HeaderProof proof2{header2, 2};
    std::vector<crosschain::HeaderProof> proofs{proof1, proof2};

    crosschain::ProofValidator validator;
    auto expected_tip = DoubleSha(header2);
    EXPECT_FALSE(validator.ValidateChain(proofs, expected_tip));
}
