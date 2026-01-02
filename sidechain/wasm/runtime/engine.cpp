#include "engine.h"

#include <array>
#include <cstdint>
#include <cstring>

namespace sidechain::wasm {

namespace {
std::vector<uint8_t> EncodeI32(int64_t value) {
    std::vector<uint8_t> out(4);
    uint32_t v = static_cast<uint32_t>(value);
    std::memcpy(out.data(), &v, sizeof(v));
    return out;
}

int64_t DecodeI32(const std::vector<uint8_t>& bytes) {
    if (bytes.size() < 4) {
        return 0;
    }
    uint32_t v = 0;
    std::memcpy(&v, bytes.data(), sizeof(v));
    return static_cast<int64_t>(v);
}
}  // namespace

bool ExecutionEngine::Push(std::vector<int64_t>& stack, int64_t value, ExecutionResult& result,
                           GasMeter& gas_meter) const {
    if (stack.size() >= kMaxStack) {
        result.error = "stack limit exceeded";
        return false;
    }
    if (!gas_meter.ConsumeMemory(sizeof(value))) {
        result.error = gas_meter.last_error();
        return false;
    }
    stack.push_back(value);
    return true;
}

bool ExecutionEngine::PopTwo(std::vector<int64_t>& stack, int64_t& a, int64_t& b,
                             ExecutionResult& result) const {
    if (stack.size() < 2) {
        result.error = "stack underflow";
        return false;
    }
    b = stack.back();
    stack.pop_back();
    a = stack.back();
    stack.pop_back();
    return true;
}

ExecutionResult ExecutionEngine::Execute(const ExecutionRequest& request,
                                         sidechain::state::StateStore& state) const {
    ExecutionResult result;
    std::string validation_error;
    if (!ValidateAssetDomain({request.domain, request.asset_id}, validation_error)) {
        result.error = validation_error;
        return result;
    }

    GasMeter gas_meter(request.gas_limit, DefaultGasSchedule());
    std::vector<int64_t> stack;
    for (const auto& instr : request.code) {
        if (!gas_meter.Consume(instr.op)) {
            result.error = gas_meter.last_error();
            break;
        }

        switch (instr.op) {
            case OpCode::Nop:
                break;
            case OpCode::ConstI32:
                if (!Push(stack, instr.immediate, result, gas_meter)) {
                    goto end_loop;
                }
                break;
            case OpCode::AddI32: {
                int64_t a = 0, b = 0;
                if (!PopTwo(stack, a, b, result)) {
                    goto end_loop;
                }
                if (!Push(stack, a + b, result, gas_meter)) {
                    goto end_loop;
                }
                break;
            }
            case OpCode::Load: {
                const auto stored = state.Get(request.domain, request.module_id,
                                              std::to_string(instr.immediate));
                if (!Push(stack, DecodeI32(stored), result, gas_meter)) {
                    goto end_loop;
                }
                break;
            }
            case OpCode::Store: {
                if (stack.empty()) {
                    result.error = "stack underflow";
                    goto end_loop;
                }
                const int64_t value = stack.back();
                stack.pop_back();
                const auto encoded = EncodeI32(value);
                if (!gas_meter.ConsumeMemory(encoded.size())) {
                    result.error = gas_meter.last_error();
                    goto end_loop;
                }
                state.Put(request.domain, request.module_id, std::to_string(instr.immediate),
                          encoded);
                ++result.state_writes;
                break;
            }
            case OpCode::ReturnTop: {
                if (stack.empty()) {
                    result.error = "stack underflow";
                    goto end_loop;
                }
                result.output = EncodeI32(stack.back());
                result.success = true;
                goto end_loop;
            }
        }
    }

    if (result.error.empty()) {
        result.success = true;
    }

end_loop:
    result.gas_used = gas_meter.used();
    return result;
}

}  // namespace sidechain::wasm
