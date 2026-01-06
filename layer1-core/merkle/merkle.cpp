#include "merkle.h"

#include "../crypto/tagged_hash.h"

#include <cstring>
#include <stdexcept>

uint256 ComputeMerkleRoot(const std::vector<Transaction>& txs)
{
    // Compute the Merkle root of transactions using tagged hashing (BIP-340 style).
    // The tree is built bottom-up by pairing transaction hashes and hashing pairs
    // until a single root hash remains. Odd-sized layers duplicate the last element
    // to maintain binary tree structure (Bitcoin-style).
    
    if (txs.empty())
        return uint256{};

    // Edge case: single transaction returns its hash directly
    if (txs.size() == 1)
        return TransactionHash(txs[0]);

    std::vector<uint256> layer;
    layer.reserve(txs.size());
    
    // Build initial layer and validate all hashes upfront
    // This is more efficient than validating in the inner loop
    for (const auto& tx : txs) {
        auto hash = TransactionHash(tx);
        if (hash.size() != 32) {
            throw std::runtime_error("invalid hash size in merkle computation");
        }
        layer.push_back(hash);
    }

    // Optimize: use single allocation for concat buffer outside loop
    uint8_t concat[64];
    
    // Build tree level by level
    while (layer.size() > 1) {
        const size_t layerSize = layer.size();
        const size_t nextSize = (layerSize + 1) / 2;
        
        std::vector<uint256> next;
        next.reserve(nextSize);
        
        for (size_t i = 0; i < layerSize; i += 2) {
            // Hashes already validated above, no need to check in hot path
            std::memcpy(concat, layer[i].data(), 32);
            
            // Handle odd-sized layer by duplicating last element
            // This follows Bitcoin's merkle tree construction algorithm
            const size_t rightIdx = (i + 1 < layerSize) ? i + 1 : i;
            std::memcpy(concat + 32, layer[rightIdx].data(), 32);
            
            // Use tagged hash for domain separation and protection against length extension
            next.push_back(tagged_hash("MERKLE", concat, sizeof(concat)));
        }
        layer.swap(next);
    }
    
    return layer.front();
}
