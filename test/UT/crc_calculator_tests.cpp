#include "msmp/crc_calculator.hpp"

#include <array>

#include <catch.hpp>

TEST_CASE("CrcCalculator", "[crc]")
{
    SECTION("crc16 for 0 = 0")
    {
        const std::vector<uint8_t> message{0};

        REQUIRE(msmp::CrcCalculator::crc16(message) == 0);
    }

    SECTION("crc16 for empty = 0")
    {
        const std::vector<uint8_t> message{};

        REQUIRE(msmp::CrcCalculator::crc16(message) == 0);
    }

    SECTION("crc16")
    {
        const std::vector<uint8_t> message{0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};

        REQUIRE(msmp::CrcCalculator::crc16(message) == 0xBB3D);
    }
}