#include "evm.h"

#include <boost/multiprecision/cpp_int.hpp>

evm_result execute(const evm_code&, const evm_state&, uint64_t)
{
    evm_result res;
    res.success = false;
    res.error = "EVM execution disabled; use mandatory WASM sidechain RPC (deploy_contract/call_contract)";
    return res;
}

std::size_t evm_word_hash::operator()(const boost::multiprecision::cpp_int& value) const noexcept
{
    return value.convert_to<std::size_t>();
}
