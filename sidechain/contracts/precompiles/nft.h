#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include <boost/multiprecision/cpp_int.hpp>
#include <leveldb/db.h>

namespace sidechain::precompiles {

using uint256 = boost::multiprecision::cpp_int;
using address = std::array<uint8_t, 20>;

struct nft_result {
    bool success{false};
    uint64_t gas_used{0};
    std::optional<address> owner; // filled for owner_of
    std::string error;
};

class nft_precompile {
public:
    explicit nft_precompile(const std::string& db_path);

    nft_result mint(const uint256& token_id, const address& to);
    nft_result transfer(const address& from, const address& to, const uint256& token_id);
    nft_result owner_of(const uint256& token_id) const;

private:
    std::string encode_token_key(const uint256& token_id) const;
    static std::string encode_address(const address& addr);
    static bool decode_address(const std::string& value, address& out);

    std::unique_ptr<leveldb::DB> db_;
};

}  // namespace sidechain::precompiles

