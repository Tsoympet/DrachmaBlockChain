#pragma once
#include <string>
#include <unordered_map>
#include <cstdint>
#include "../tx/txout.h"

struct UTXO {
    TxOut out;
    uint32_t height;
    bool coinbase;

    UTXO() : height(0), coinbase(false) {}
    UTXO(const TxOut& o, uint32_t h, bool cb)
        : out(o), height(h), coinbase(cb) {}
};

using OutPointKey = std::string; // txid:index

inline OutPointKey MakeOutPointKey(const std::string& txid, uint32_t index) {
    return txid + ":" + std::to_string(index);
}
