#include "proof_validator.h"

#include <openssl/sha.h>
#include <algorithm>
#include <stdexcept>

namespace crosschain {

static std::array<uint8_t, 32> DoubleSha(const std::array<uint8_t, 80>& header)
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

static std::array<uint8_t, 32> ExtractPrevHash(const std::array<uint8_t, 80>& header)
{
    std::array<uint8_t, 32> prev{};
    std::copy_n(header.data() + 4, prev.size(), prev.data());
    return prev;
}

bool ProofValidator::ValidateChain(const std::vector<HeaderProof>& proofs, const std::array<uint8_t, 32>& expectedTip)
{
    if (proofs.empty()) return false;

    for (size_t i = 1; i < proofs.size(); ++i) {
        if (proofs[i].height <= proofs[i - 1].height) {
            return false;
        }
        auto prev_hash = ExtractPrevHash(proofs[i].header);
        auto expected_prev_hash = DoubleSha(proofs[i - 1].header);
        if (prev_hash != expected_prev_hash) {
            return false;
        }
    }

    if (expectedTip == std::array<uint8_t, 32>{}) {
        return true; // allow bootstrap when genesis tip is unknown
    }
    return DoubleSha(proofs.back().header) == expectedTip;
}

} // namespace crosschain
