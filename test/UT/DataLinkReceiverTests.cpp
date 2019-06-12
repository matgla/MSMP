#include <cstdint>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <eul/logger/logger_factory.hpp>
#include <eul/signals/signal.hpp>

#include "msmp/layers/datalink/receiver/datalink_receiver.hpp"
#include "msmp/types.hpp"

#include "test/UT/stubs/StandardErrorStreamStub.hpp"
#include "test/UT/stubs/TimeStub.hpp"
#include "test/UT/stubs/TimerManagerStub.hpp"

namespace msmp
{
namespace layers
{
namespace datalink
{
namespace receiver
{

struct SmallBufferConfiguration
{
    static constexpr std::size_t max_payload_size = 5;
};

class DataLinkReceiverShould : public ::testing::Test
{
public:
    DataLinkReceiverShould() : logger_factory_(time_)
    {
    }

protected:
    stubs::TimeStub time_;
    eul::logger::logger_factory logger_factory_;
    std::vector<uint8_t> buffer_;

    ErrorCode error_code_;
};


TEST_F(DataLinkReceiverShould, ReportBufferOverflow)
{
    DataLinkReceiver sut_(logger_factory_);

    DataLinkReceiver::OnFailureSlot on_failure([this](const StreamType& stream, const ErrorCode ec) {
        buffer_.insert(buffer_.end(), stream.begin(), stream.end());
        error_code_ = ec;
    });
    sut_.doOnFailure(on_failure);
    sut_.receiveByte(static_cast<uint8_t>(ControlByte::StartFrame));
    for (std::size_t i = 0; i < configuration::Configuration::max_payload_size + 10; i++)
    {
        sut_.receiveByte(1);
    }

    EXPECT_EQ(error_code_, ErrorCode::MessageBufferOverflow);
}

TEST_F(DataLinkReceiverShould, ReceiveData)
{
    DataLinkReceiver sut_(logger_factory_);

    DataLinkReceiver::OnDataSlot on_data([this](const StreamType& stream) {
        buffer_.insert(buffer_.end(), stream.begin(), stream.end());
    });
    sut_.doOnData(on_data);

    sut_.receiveByte(static_cast<uint8_t>(ControlByte::StartFrame));
    sut_.receiveByte(1);
    sut_.receiveByte(2);
    sut_.receiveByte(3);
    sut_.receiveByte(4);
    sut_.receiveByte(static_cast<uint8_t>(ControlByte::StartFrame));

    EXPECT_THAT(buffer_, ::testing::ElementsAreArray({1, 2, 3, 4}));
}

TEST_F(DataLinkReceiverShould, ReceiveStuffedData)
{
    DataLinkReceiver sut_(logger_factory_);
    DataLinkReceiver::OnDataSlot on_data([this](const StreamType& stream) {
        buffer_.insert(buffer_.end(), stream.begin(), stream.end());
    });
    sut_.doOnData(on_data);
    sut_.receiveByte(static_cast<uint8_t>(ControlByte::StartFrame));
    sut_.receiveByte(static_cast<uint8_t>(ControlByte::EscapeCode));
    sut_.receiveByte(2);
    sut_.receiveByte(static_cast<uint8_t>(ControlByte::EscapeCode));
    sut_.receiveByte(static_cast<uint8_t>(ControlByte::EscapeCode));
    sut_.receiveByte(static_cast<uint8_t>(ControlByte::EscapeCode));
    sut_.receiveByte(static_cast<uint8_t>(ControlByte::StartFrame));

    sut_.receiveByte(static_cast<uint8_t>(ControlByte::StartFrame));

    EXPECT_THAT(buffer_, ::testing::ElementsAreArray({2, static_cast<int>(ControlByte::EscapeCode),
                                                      static_cast<int>(ControlByte::StartFrame)}));
}

} // namespace receiver
} // namespace datalink
} // namespace layers
} // namespace msmp
