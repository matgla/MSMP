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

    SECTION("crc16 for 0xabcd = 0xa5be")
    {
        const std::vector<uint8_t> message{0xA, 0xB, 0xC, 0xD};

        REQUIRE(msmp::CrcCalculator::crc16(message) == 0xA5BE);
    }
}