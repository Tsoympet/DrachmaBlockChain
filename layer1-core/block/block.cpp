#include "block.h"

uint256 BlockHash(const BlockHeader& header) {
    return TaggedHash("BLOCK",
        reinterpret_cast<const uint8_t*>(&header),
        sizeof(BlockHeader)
    );
}

