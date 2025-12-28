#include <gtest/gtest.h>

#include "../../layer1-core/crypto/schnorr.h"

#include <array>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::vector<uint8_t> hex_to_bytes(const std::string& hex)
{
    std::vector<uint8_t> out;
    out.reserve(hex.size() / 2);
    for (size_t i = 0; i + 1 < hex.size(); i += 2) {
        auto byte = static_cast<uint8_t>(std::stoul(hex.substr(i, 2), nullptr, 16));
        out.push_back(byte);
    }
    return out;
}

std::array<uint8_t, 32> to_array32(const std::string& hex)
{
    auto bytes = hex_to_bytes(hex);
    if (bytes.size() != 32) {
        throw std::runtime_error("expected 32-byte hex");
    }
    std::array<uint8_t, 32> out{};
    std::copy(bytes.begin(), bytes.end(), out.begin());
    return out;
}

std::array<uint8_t, 64> to_array64(const std::string& hex)
{
    auto bytes = hex_to_bytes(hex);
    if (bytes.size() != 64) {
        throw std::runtime_error("expected 64-byte hex");
    }
    std::array<uint8_t, 64> out{};
    std::copy(bytes.begin(), bytes.end(), out.begin());
    return out;
}

} // namespace

TEST(SchnorrBip340, Vector0MatchesReference)
{
    const auto seckey = to_array32("0000000000000000000000000000000000000000000000000000000000000003");
    const auto pubkey = to_array32("F9308A019258C31049344F85F89D5229B531C845836F99B08601F113BCE036F9");
    const auto msg = to_array32("0000000000000000000000000000000000000000000000000000000000000000");
    const auto aux = to_array32("0000000000000000000000000000000000000000000000000000000000000000");
    const auto expected_sig = to_array64("E907831F80848D1069A5371B402410364BDF1C5F8307B0084C55F1CE2DCA821525F66A4A85EA8B71E482A74F382D2CE5EBEEE8FDB2172F477DF4900D310536C0");

    std::array<uint8_t, 64> sig{};
    ASSERT_TRUE(schnorr_sign_with_aux(seckey.data(), msg.data(), aux.data(), sig.data()));
    EXPECT_EQ(sig, expected_sig);
    EXPECT_TRUE(schnorr_verify(pubkey.data(), msg.data(), sig.data()));

    auto tampered_sig = sig;
    tampered_sig[0] ^= 0x01;
    EXPECT_FALSE(schnorr_verify(pubkey.data(), msg.data(), tampered_sig.data()));
}

TEST(SchnorrBip340, Vector1MatchesReference)
{
    const auto seckey = to_array32("B7E151628AED2A6ABF7158809CF4F3C762E7160F38B4DA56A784D9045190CFEF");
    const auto pubkey = to_array32("DFF1D77F2A671C5F36183726DB2341BE58FEAE1DA2DECED843240F7B502BA659");
    const auto msg = to_array32("243F6A8885A308D313198A2E03707344A4093822299F31D0082EFA98EC4E6C89");
    const auto aux = to_array32("0000000000000000000000000000000000000000000000000000000000000001");
    const auto expected_sig = to_array64("6896BD60EEAE296DB48A229FF71DFE071BDE413E6D43F917DC8DCF8C78DE33418906D11AC976ABCCB20B091292BFF4EA897EFCB639EA871CFA95F6DE339E4B0A");

    std::array<uint8_t, 64> sig{};
    ASSERT_TRUE(schnorr_sign_with_aux(seckey.data(), msg.data(), aux.data(), sig.data()));
    EXPECT_EQ(sig, expected_sig);
    EXPECT_TRUE(schnorr_verify(pubkey.data(), msg.data(), sig.data()));
}

TEST(SchnorrBip340, NonceReuseIsRejected)
{
    const auto seckey = to_array32("C90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B14E5C9");
    const auto pubkey = to_array32("DD308AFEC5777E13121FA72B9CC1B7CC0139715309B086C960E18FD969774EB8");
    const auto aux = to_array32("C87AA53824B4D7AE2EB035A2B5BBBCCC080E76CDC6D1692C4B0B62D798E6D906");
    const auto msg1 = to_array32("7E2D58D8B3BCDF1ABADEC7829054F90DDA9805AAB56C77333024B9D0A508B75C");
    const auto msg2 = to_array32("5831AAEED7B44BB74E5EAB94BA9D4294C49BCF2A60728D8B4C200F50DD313C1B");

    std::array<uint8_t, 64> sig1{};
    std::array<uint8_t, 64> sig2{};
    ASSERT_TRUE(schnorr_sign_with_aux(seckey.data(), msg1.data(), aux.data(), sig1.data()));
    ASSERT_TRUE(schnorr_sign_with_aux(seckey.data(), msg2.data(), aux.data(), sig2.data()));
    EXPECT_NE(sig1, sig2);
    EXPECT_TRUE(schnorr_verify(pubkey.data(), msg1.data(), sig1.data()));
    EXPECT_FALSE(schnorr_verify(pubkey.data(), msg2.data(), sig1.data()));
}

TEST(SchnorrBip340, InvalidSecretsFail)
{
    std::array<uint8_t, 32> zero{};
    std::array<uint8_t, 32> msg{};
    std::array<uint8_t, 64> sig{};
    EXPECT_FALSE(schnorr_sign_with_aux(zero.data(), msg.data(), nullptr, sig.data()));
}
