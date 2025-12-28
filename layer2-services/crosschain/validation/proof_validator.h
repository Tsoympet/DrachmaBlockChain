#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace crosschain {

struct HeaderProof {
    std::array<uint8_t, 80> header;
    uint32_t height;
};

class ProofValidator {
public:
    bool ValidateChain(const std::vector<HeaderProof>& proofs, const std::array<uint8_t, 32>& expectedTip);
};

} // namespace crosschain

