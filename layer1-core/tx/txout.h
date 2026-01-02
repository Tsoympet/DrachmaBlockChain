#pragma once
#include <vector>
#include <cstdint>

struct TxOut {
    uint64_t value;                 // smallest unit
    std::vector<uint8_t> scriptPubKey;
    uint8_t assetId{1};

    TxOut() : value(0) {}
    TxOut(uint64_t v, const std::vector<uint8_t>& script, uint8_t asset = 1)
        : value(v), scriptPubKey(script), assetId(asset) {}
};
