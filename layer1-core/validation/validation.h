#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "../tx/transaction.h"
#include "../chainstate/coins.h"
#include "../block/block.h"     // χρησιμοποιεί το υπάρχον Block/BlockHeader
#include "../consensus/params.h"

namespace validation {

bool CheckTransaction(const Transaction& tx, std::string& error);

bool CheckBlockStructure(const Block& block, std::string& error);
bool CheckMerkleRoot(const Block& block, std::string& error);
bool CheckProofOfWork(const BlockHeader& header,
                      const ConsensusParams& params,
                      std::string& error);

bool ConnectBlock(const Block& block,
                  Chainstate& chainstate,
                  uint32_t height,
                  BlockUndo& undo,
                  std::string& error);

} // namespace validation
