#pragma once
#include <vector>
#include "../crypto/tagged_hash.h"
#include "../tx/transaction.h"

uint256 ComputeMerkleRoot(const std::vector<Transaction>& txs);
