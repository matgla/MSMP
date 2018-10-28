#include "msmp/frame_receiver.hpp"

#include <catch.hpp>

#include <boost/di.hpp>

#include <eul/mpl/mixin/object.hpp>


namespace msmp
{

struct DataReceiver
{
    DataReceiver(std::vector<uint8_t>& buffer)
        : buffer_(buffer)
    {
    }

    void receive(const DataStream& payload)
    {
        buffer_.insert(buffer_.end(), payload.begin(), payload.end());
    }

private:
    std::vector<uint8_t>& buffer_;
};

TEST_CASE("ReceiverInterface should", "[frame_receiver]")
{
    using namespace eul::mpl::mixin;
    SECTION("Delegate bytes to receiver implementation")
    {
        std::vector<uint8_t> buffer;
        auto receiver = object{interface<msmp::IFrameReceiver>{}, DataReceiver{buffer}};

        const uint8_t data[] = {0xB, 0xA, 0xB, 0xE};
        receiver.receive_data(data);

        REQUIRE(buffer == std::vector<uint8_t>{0xB, 0xA, 0xB, 0xE});

        const uint8_t data2[] = {0xC, 0xA, 0xF, 0xE};
        receiver.receive_data(data2);

        REQUIRE(buffer == std::vector<uint8_t>{0xB, 0xA, 0xB, 0xE, 0xC, 0xA, 0xF, 0xE});
    }
}

} // namespace msmp
