#include "proof_validator.h"

#include <openssl/sha.h>
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

bool ProofValidator::ValidateChain(const std::vector<HeaderProof>& proofs, const std::array<uint8_t, 32>& expectedTip)
{
    if (proofs.empty()) return false;
    std::array<uint8_t, 32> prev = DoubleSha(proofs.front().header);

    for (size_t i = 1; i < proofs.size(); ++i) {
        if (proofs[i].height <= proofs[i - 1].height) {
            return false;
        }
        auto h = DoubleSha(proofs[i].header);
        if (h != prev) {
            return false;
        }
        prev = h;
    }

    if (expectedTip == std::array<uint8_t, 32>{}) {
        return true; // allow bootstrap when genesis tip is unknown
    }
    return prev == expectedTip;
}

} // namespace crosschain

