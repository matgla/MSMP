#include <cstdint>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <eul/logger/logger_factory.hpp>

#include "msmp/data_link_receiver.hpp"

#include "test/UT/stubs/StandardErrorStreamStub.hpp"
#include "test/UT/stubs/TimeStub.hpp"
#include "test/UT/stubs/TimerManagerStub.hpp"

namespace msmp
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

    using DataLinkReceiverType = DataLinkReceiver<SmallBufferConfiguration>;
    DataLinkReceiverType::ErrorCode error_code_;
};


TEST_F(DataLinkReceiverShould, ReportBufferOverflow)
{
    DataLinkReceiverType sut_(logger_factory_);
    sut_.on_failure(
        [this](const DataLinkReceiverType::StreamType& stream, const DataLinkReceiverType::ErrorCode ec) {
            buffer_.insert(buffer_.end(), stream.begin(), stream.end());
            error_code_ = ec;
        });
    sut_.receive_byte(static_cast<uint8_t>(ControlByte::StartFrame));
    sut_.receive_byte(1);
    sut_.receive_byte(1);
    sut_.receive_byte(1);
    sut_.receive_byte(1);
    sut_.receive_byte(1);
    sut_.receive_byte(1);

    EXPECT_THAT(buffer_, ::testing::ElementsAreArray({1, 1, 1, 1, 1}));

    EXPECT_EQ(error_code_, DataLinkReceiverType::ErrorCode::MessageBufferOverflow);
}

TEST_F(DataLinkReceiverShould, ReceiveData)
{
    DataLinkReceiverType sut_(logger_factory_);
    sut_.on_data([this](const DataLinkReceiverType::StreamType& stream) {
        buffer_.insert(buffer_.end(), stream.begin(), stream.end());
    });
    sut_.receive_byte(static_cast<uint8_t>(ControlByte::StartFrame));
    sut_.receive_byte(1);
    sut_.receive_byte(2);
    sut_.receive_byte(3);
    sut_.receive_byte(4);
    sut_.receive_byte(static_cast<uint8_t>(ControlByte::StartFrame));

    EXPECT_THAT(buffer_, ::testing::ElementsAreArray({1, 2, 3, 4}));

    EXPECT_EQ(error_code_, DataLinkReceiverType::ErrorCode::MessageBufferOverflow);
}

TEST_F(DataLinkReceiverShould, ReceiveStuffedData)
{
    DataLinkReceiverType sut_(logger_factory_);
    sut_.on_data([this](const DataLinkReceiverType::StreamType& stream) {
        buffer_.insert(buffer_.end(), stream.begin(), stream.end());
    });
    sut_.receive_byte(static_cast<uint8_t>(ControlByte::StartFrame));
    sut_.receive_byte(static_cast<uint8_t>(ControlByte::EscapeCode));
    sut_.receive_byte(2);
    sut_.receive_byte(static_cast<uint8_t>(ControlByte::EscapeCode));
    sut_.receive_byte(static_cast<uint8_t>(ControlByte::EscapeCode));
    sut_.receive_byte(static_cast<uint8_t>(ControlByte::EscapeCode));
    sut_.receive_byte(static_cast<uint8_t>(ControlByte::StartFrame));

    sut_.receive_byte(static_cast<uint8_t>(ControlByte::StartFrame));

    EXPECT_THAT(buffer_, ::testing::ElementsAreArray({2, static_cast<int>(ControlByte::EscapeCode),
                                                      static_cast<int>(ControlByte::StartFrame)}));
}

} // namespace msmp
