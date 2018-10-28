// #include "msmp/frame_sender.hpp"

// #include <array>

// #include <catch.hpp>

// constexpr uint8_t START_BYTE  = 0x1F;
// constexpr uint8_t ESCAPE_BYTE = 0x1B;

// class WriterStub
// {
// public:
//     void write(const gsl::span<const uint8_t>& data)
//     {
//         data_.insert(data_.end(), data.begin(), data.end());
//     }

//     const std::vector<uint8_t>& data() const
//     {
//         return data_;
//     }

// private:
//     std::vector<uint8_t> data_;
// };

// TEST_CASE("Sender should", "[frame_sender]")
// {
//     WriterStub writer;
//     msmp::FrameSender<16> sut(
//         [&writer](const gsl::span<const uint8_t>& data) { writer.write(data); });

//     SECTION("send data correctly")
//     {
//         const std::vector<uint8_t> message{0xA, 0xB, 0xC, 0xD};
//         const std::vector<uint8_t> expected_message{START_BYTE, 0xA, 0xB, 0xC, 0xD, 0xB6, 0xDF};
//         sut.send(message);
//         REQUIRE(writer.data() == expected_message);
//     }

//     SECTION("should escape special symbols")
//     {
//         const std::vector<uint8_t> message{0xA, START_BYTE, 0xC, ESCAPE_BYTE};
//         const std::vector<uint8_t> expected_message{
//             START_BYTE, 0xA, ESCAPE_BYTE, START_BYTE, 0xC, ESCAPE_BYTE, ESCAPE_BYTE, 0xE9, 0x84};
//         sut.send(message);
//         REQUIRE(writer.data() == expected_message);
//     }

//     SECTION("should split to frames")
//     {
//         msmp::FrameSender<16, 4> sut([&writer](const gsl::span<const uint8_t>& data) { writer.write(data); });
//         // clang-format off
//         const std::vector<uint8_t> message{0xA, START_BYTE, 0xC, 0xA,
//                                            0xB, ESCAPE_BYTE, 0xA, 0xB,
//                                            0xB};
//         const std::vector<uint8_t> expected_message{
//             START_BYTE, 0xA, ESCAPE_BYTE, START_BYTE, 0xC, 0x7B, 0xEA, // first frame
//             START_BYTE, 0xA, 0xB, ESCAPE_BYTE, ESCAPE_BYTE, 0x38, 0xE1, // second frame
//             START_BYTE, 0xA, 0xB, 0xB, 0x66, 0xF5}; // third frame
//         // clang-format on

//         sut.send(message);
//         REQUIRE(writer.data() == expected_message);
//     }
// }