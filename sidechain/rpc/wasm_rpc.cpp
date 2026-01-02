#include "wasm_rpc.h"

#include "../wasm/validator/validator.h"

namespace sidechain::rpc {

using sidechain::wasm::ExecutionDomain;
using sidechain::wasm::ExecutionRequest;
using sidechain::wasm::ExecutionResult;
using sidechain::wasm::Instruction;

namespace {
sidechain::wasm::ExecutionResult DomainViolation() {
    ExecutionResult res;
    res.error = "asset/domain violation";
    return res;
}

bool Validate(ExecutionDomain domain, uint8_t asset, std::string& error) {
    return sidechain::wasm::ValidateAssetDomain({domain, asset}, error);
}
}  // namespace

WasmRpcService::WasmRpcService(sidechain::wasm::ExecutionEngine& engine,
                               sidechain::state::StateStore& state)
    : engine_(engine), state_(state) {}

ExecutionResult WasmRpcService::DeployContract(const DeployRequest& request) {
    std::string error;
    if (!Validate(ExecutionDomain::SmartContract, request.asset_id, error)) {
        ExecutionResult res;
        res.error = error;
        return res;
    }

    ExecutionRequest exec;
    exec.domain = ExecutionDomain::SmartContract;
    exec.asset_id = request.asset_id;
    exec.module_id = request.contract_id;
    exec.code = request.init_code;
    exec.gas_limit = request.gas_limit;
    return engine_.Execute(exec, state_);
}

ExecutionResult WasmRpcService::CallContract(const ContractCall& request) {
    std::string error;
    if (!Validate(ExecutionDomain::SmartContract, request.asset_id, error)) {
        ExecutionResult res;
        res.error = error;
        return res;
    }

    ExecutionRequest exec;
    exec.domain = ExecutionDomain::SmartContract;
    exec.asset_id = request.asset_id;
    exec.module_id = request.contract_id;
    exec.code = request.code;
    exec.gas_limit = request.gas_limit;
    return engine_.Execute(exec, state_);
}

ExecutionResult WasmRpcService::MintNft(const MintNftRequest& request) {
    std::string error;
    if (!Validate(ExecutionDomain::NFT, request.asset_id, error)) {
        ExecutionResult res;
        res.error = error;
        return res;
    }
    ExecutionResult res;
    const std::string module = "nft";
    if (state_.Exists(ExecutionDomain::NFT, module, request.token_id)) {
        res.error = "token exists";
        return res;
    }
    std::vector<uint8_t> owner_bytes(request.owner.begin(), request.owner.end());
    std::vector<uint8_t> meta_bytes(request.metadata_hash.begin(), request.metadata_hash.end());
    state_.Put(ExecutionDomain::NFT, module, request.token_id, owner_bytes);
    state_.Put(ExecutionDomain::NFT, module + ":meta", request.token_id, meta_bytes);
    res.success = true;
    res.gas_used = request.gas_limit;
    return res;
}

ExecutionResult WasmRpcService::TransferNft(const TransferNftRequest& request) {
    std::string error;
    if (!Validate(ExecutionDomain::NFT, request.asset_id, error)) {
        ExecutionResult res;
        res.error = error;
        return res;
    }
    ExecutionResult res;
    const std::string module = "nft";
    if (!state_.Exists(ExecutionDomain::NFT, module, request.token_id)) {
        res.error = "token missing";
        return res;
    }
    const auto owner_bytes = state_.Get(ExecutionDomain::NFT, module, request.token_id);
    const std::string current_owner(owner_bytes.begin(), owner_bytes.end());
    if (current_owner != request.from) {
        res.error = "ownership mismatch";
        return res;
    }
    std::vector<uint8_t> new_owner(request.to.begin(), request.to.end());
    state_.Put(ExecutionDomain::NFT, module, request.token_id, new_owner);
    res.success = true;
    res.gas_used = request.gas_limit;
    return res;
}

ExecutionResult WasmRpcService::CallDapp(const DappCall& request) {
    std::string error;
    if (!Validate(ExecutionDomain::Dapp, request.asset_id, error)) {
        ExecutionResult res;
        res.error = error;
        return res;
    }
    ExecutionRequest exec;
    exec.domain = ExecutionDomain::Dapp;
    exec.asset_id = request.asset_id;
    exec.module_id = request.app_id;
    exec.code = request.code;
    exec.gas_limit = request.gas_limit;
    return engine_.Execute(exec, state_);
}

}  // namespace sidechain::rpc
