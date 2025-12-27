#include "../../layer1-core/merkle/merkle.h"
#include <vector>
#include <cstdint>
#include <cstring>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size == 0)
        return 0;

    // Build a variable number of fake transactions by slicing the buffer.
    std::vector<Transaction> txs;
    size_t offset = 0;
    while (offset < size) {
        Transaction tx;
        TxOut out;
        out.value = data[offset] * 1000ULL;
        out.scriptPubKey.assign(data + offset, data + std::min<size_t>(size, offset + 32));
        tx.vout.push_back(out);
        txs.push_back(tx);
        offset += std::max<size_t>(1, out.scriptPubKey.size());
    }

    (void)ComputeMerkleRoot(txs);
    return 0;
}

int main(int argc, char** argv)
{
    if (argc < 2)
        return 0;
    std::vector<uint8_t> buf(argv[1], argv[1] + strlen(argv[1]));
    LLVMFuzzerTestOneInput(buf.data(), buf.size());
    return 0;
}
