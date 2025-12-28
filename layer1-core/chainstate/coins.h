#pragma once
#include <unordered_map>
#include <string>
#include <cstdint>
#include "utxo.h"
#include "undo.h"
#include "../tx/transaction.h"

class CoinsView {
public:
    bool HaveUTXO(const OutPointKey& key) const;
    const UTXO& GetUTXO(const OutPointKey& key) const;
    void AddUTXO(const OutPointKey& key, const UTXO& utxo);
    void RemoveUTXO(const OutPointKey& key);

private:
    std::unordered_map<OutPointKey, UTXO> mapUTXO;
};

class Chainstate {
public:
    Chainstate();

    bool ApplyTransaction(const Transaction& tx,
                          uint32_t height,
                          BlockUndo& undo,
                          std::string& error);

    void ApplyBlock(const std::vector<Transaction>& txs,
                    uint32_t height,
                    BlockUndo& undo);

    void DisconnectBlock(const BlockUndo& undo);

    const CoinsView& View() const { return coins; }

private:
    CoinsView coins;
    static constexpr uint32_t COINBASE_MATURITY = 100;
};

