#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <CRC.h>

#include <eul/logger/logger_factory.hpp>

#include "msmp/transport_transceiver.hpp"
#include "msmp/layers/transport/receiver/transport_receiver.hpp"
#include "msmp/transport_transmitter.hpp"
#include "msmp/messages/control/ack.hpp"

#include "test/UT/stubs/StandardErrorStreamStub.hpp"
#include "test/UT/stubs/TimeStub.hpp"
#include "test/UT/stubs/TimerManagerStub.hpp"
#include "test/UT/stubs/DataLinkTransmitterStub.hpp"
#include "test/UT/stubs/DataLinkReceiverStub.hpp"


namespace msmp
{

class TransportTransceiverTests : public ::testing::Test
{
public:
    TransportTransceiverTests()
        : logger_factory_(time_)
        , transport_transmitter_(logger_factory_, datalink_transmitter_, time_)
        , transport_receiver_(logger_factory_, datalink_receiver_)
    {
        datalink_transmitter_.enable_auto_emitting();
    }

    std::vector<uint8_t> generate_ack(uint8_t transaction_id)
    {
        std::vector<uint8_t> payload {1, 0};
        auto ack = messages::control::Ack{.transaction_id = transaction_id}.serialize();
        std::copy(ack.begin(), ack.end(), std::back_inserter(payload));

        const uint32_t crc = CRC::Calculate(payload.data(), payload.size(), CRC::CRC_32());
        payload.push_back((crc >> 24) & 0xff);
        payload.push_back((crc >> 16) & 0xff);
        payload.push_back((crc >> 8) & 0xff);
        payload.push_back(crc & 0xff);

        return payload;
    }

    std::vector<uint8_t> generate_nack(uint8_t transaction_id)
    {
        std::vector<uint8_t> payload {1, 0};
        auto nack = messages::control::Nack{.transaction_id = transaction_id, .reason = messages::control::Nack::Reason::CrcMismatch}.serialize();
        std::copy(nack.begin(), nack.end(), std::back_inserter(payload));

        const uint32_t crc = CRC::Calculate(payload.data(), payload.size(), CRC::CRC_32());
        payload.push_back((crc >> 24) & 0xff);
        payload.push_back((crc >> 16) & 0xff);
        payload.push_back((crc >> 8) & 0xff);
        payload.push_back(crc & 0xff);

        return payload;
    }
protected:
    stubs::TimeStub time_;
    eul::logger::logger_factory logger_factory_;

    ::test::stubs::DataLinkTransmitterStub datalink_transmitter_;
    test::stubs::DataLinkReceiverStub datalink_receiver_;
    TransportTransmitter<::test::stubs::DataLinkTransmitterStub, configuration::Configuration> transport_transmitter_;
    layers::transport::receiver::TransportReceiver transport_receiver_;
};

TEST_F(TransportTransceiverTests, SendMessages)
{
    TransportTransceiver sut(logger_factory_, transport_receiver_, transport_transmitter_);
    sut.send(std::vector<uint8_t>{1, 2, 3, 4, 5});
    sut.send(std::vector<uint8_t>{3, 4});

    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(datalink_transmitter_.get_buffer(), ::testing::ElementsAreArray({
        2,
        1,
        1, 2, 3, 4, 5,
        0x40, 0x86, 0x73, 0x1b
    }));

    auto ack1 = generate_ack(1);
    datalink_transmitter_.clear_buffer();

    datalink_receiver_.receive(ack1);
    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(datalink_transmitter_.get_buffer(), ::testing::ElementsAreArray({
        2,
        2,
        3, 4,
        0xa4, 0x89, 0x54, 0x23
    }));
}

TEST_F(TransportTransceiverTests, ReceiveMessages)
{
    TransportTransceiver sut(logger_factory_, transport_receiver_, transport_transmitter_);
    std::vector<uint8_t> buffer;
    sut.on_data([&buffer](const StreamType& stream)
    {
        std::copy(stream.begin(), stream.end(), std::back_inserter(buffer));
    });

    const std::vector<uint8_t> data{
        2,
        1,
        1, 2, 3, 4, 5,
        0x40, 0x86, 0x73, 0x1b
    };

    datalink_receiver_.receive(data);
    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(buffer, ::testing::ElementsAreArray({1, 2, 3, 4, 5}));
}

TEST_F(TransportTransceiverTests, RespondNackForCrcMismatch)
{
    TransportTransceiver sut(logger_factory_, transport_receiver_, transport_transmitter_);

    const std::vector<uint8_t> data{
        1, 0,
        0xff,
        0x75, 0xc0, 0xcc, 0x33
    };

    datalink_receiver_.receive(data);

    configuration::Configuration::execution_queue.run();
    EXPECT_THAT(datalink_transmitter_.get_buffer(), ::testing::ElementsAreArray({
        2, 1,
        static_cast<int>(messages::control::Nack::id),
        0, static_cast<int>(messages::control::Nack::Reason::CrcMismatch),
        0xeb, 0x92, 0xc8, 0x3
    }));
}

TEST_F(TransportTransceiverTests, RespondNackForWrongMessageType)
{
    TransportTransceiver sut(logger_factory_, transport_receiver_, transport_transmitter_);

    const std::vector<uint8_t> data{
        7, 0,
        0xff,
        0xd7, 0x0c, 0x20, 0x1a
    };

    datalink_receiver_.receive(data);

    configuration::Configuration::execution_queue.run();
    EXPECT_THAT(datalink_transmitter_.get_buffer(), ::testing::ElementsAreArray({
        2, 1,
        static_cast<int>(messages::control::Nack::id),
        0, static_cast<int>(messages::control::Nack::Reason::WrongMessageType),
        0x72, 0x9b, 0x99, 0xb9
    }));
}

TEST_F(TransportTransceiverTests, RetransmitAfterNack)
{
    TransportTransceiver sut(logger_factory_, transport_receiver_, transport_transmitter_);
    bool success = false;
    bool failure = false;
    sut.send(std::vector<uint8_t>{1, 2, 3, 4, 5}, [&success]{ success = true; }, [&failure]{ failure = true;});
    sut.send(std::vector<uint8_t>{3, 4});

    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(datalink_transmitter_.get_buffer(), ::testing::ElementsAreArray({
        2,
        1,
        1, 2, 3, 4, 5,
        0x40, 0x86, 0x73, 0x1b
    }));

    auto nack = generate_nack(1);
    datalink_transmitter_.clear_buffer();
    datalink_receiver_.receive(nack);
    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(datalink_transmitter_.get_buffer(), ::testing::ElementsAreArray({
        2,
        1,
        1, 2, 3, 4, 5,
        0x40, 0x86, 0x73, 0x1b
    }));
    auto ack = generate_ack(1);
    EXPECT_FALSE(failure);
    EXPECT_FALSE(success);
    datalink_transmitter_.clear_buffer();
    datalink_receiver_.receive(ack);
    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(datalink_transmitter_.get_buffer(), ::testing::ElementsAreArray({
        2,
        2,
        3, 4,
        0xa4, 0x89, 0x54, 0x23
    }));

    EXPECT_FALSE(failure);
    EXPECT_TRUE(success);
}

TEST_F(TransportTransceiverTests, NotifyFailureWhenRetransmissionExceeded)
{
    TransportTransceiver sut(logger_factory_, transport_receiver_, transport_transmitter_);
    bool success = false;
    bool failure = false;
    sut.send(std::vector<uint8_t>{1, 2, 3, 4, 5}, [&success]{ success = true; }, [&failure](){ failure = true;});
    sut.send(std::vector<uint8_t>{3, 4});

    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(datalink_transmitter_.get_buffer(), ::testing::ElementsAreArray({
        2,
        1,
        1, 2, 3, 4, 5,
        0x40, 0x86, 0x73, 0x1b
    }));

    for (std::size_t i = 0; i < configuration::Configuration::max_retransmission_tries + 1; ++i)
    {
        auto nack = generate_nack(1);
        datalink_receiver_.receive(nack);
        configuration::Configuration::execution_queue.run();
    }
    EXPECT_TRUE(failure);
    EXPECT_FALSE(success);
}

TEST_F(TransportTransceiverTests, RetransmitAfterTimeout)
{
    TransportTransceiver sut(logger_factory_, transport_receiver_, transport_transmitter_);
    bool success = false;
    bool failure = false;
    sut.send(std::vector<uint8_t>{1, 2}, [&success]{ success = true; }, [&failure](){ failure = true;});
    configuration::Configuration::execution_queue.run();
    EXPECT_THAT(datalink_transmitter_.get_buffer(), ::testing::ElementsAreArray({
        2,
        1,
        1, 2,
        0x7d, 0x9a, 0x2d, 0xcd
    }));

    datalink_transmitter_.clear_buffer();
    time_ += configuration::Configuration::timeout_for_transmission;
    time_ += std::chrono::milliseconds(1);

    configuration::Configuration::timer_manager.run();
    configuration::Configuration::execution_queue.run();
    EXPECT_THAT(datalink_transmitter_.get_buffer(), ::testing::ElementsAreArray({
        2,
        1,
        1, 2,
        0x7d, 0x9a, 0x2d, 0xcd
    }));
}

} // namespace msmp
