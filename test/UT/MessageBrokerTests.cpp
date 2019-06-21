
// #include <gmock/gmock.h>
// #include <gtest/gtest.h>

// #include <eul/logger/logger_factory.hpp>
// #include <eul/timer/timer_manager.hpp>

// #include "msmp/message_broker.hpp"
// #include "msmp/configuration/configuration.hpp"

// #include "test/UT/stubs/StandardErrorStreamStub.hpp"
// #include "test/UT/stubs/TimeStub.hpp"
// #include "test/UT/stubs/TimerManagerStub.hpp"
// #include "test/UT/stubs/TransceiverStub.hpp"

// namespace msmp
// {

// class ExampleMessage
// {
// public:
//     ExampleMessage(const std::vector<uint8_t>& payload) : payload_(payload)
//     {
//     }

//     std::vector<uint8_t> serialize() const
//     {
//         return payload_;
//     }
// private:
//     std::vector<uint8_t> payload_;
// };

// class MessageBrokerShould : public ::testing::Test
// {
// public:
//     MessageBrokerShould() : logger_factory_(time_)
//     {
//     }

// protected:
//     stubs::TimeStub time_;
//     eul::logger::logger_factory logger_factory_;
//     test::stubs::TransceiverStub transceiver_;
// };

// TEST_F(MessageBrokerShould, handleMessage)
// {
//     MessageBroker sut(logger_factory_, transceiver_);
//     std::vector<uint8_t> buffer1;
//     std::vector<uint8_t> buffer2;
//     sut.register_handler(0, [&buffer1](const StreamType& stream) {
//         std::copy(stream.begin(), stream.end(), std::back_inserter(buffer1));
//     });

//     sut.register_handler(1, [&buffer2](const StreamType& stream) {
//         std::copy(stream.begin(), stream.end(), std::back_inserter(buffer2));
//     });

//     transceiver_.receiveData(std::vector<uint8_t>{0, 1, 2, 3});
//     EXPECT_THAT(buffer1, ::testing::ElementsAreArray({0, 1, 2, 3}));
//     EXPECT_THAT(buffer2, ::testing::IsEmpty());

//     transceiver_.receiveData(std::vector<uint8_t>{1, 2, 2, 1});
//     EXPECT_THAT(buffer1, ::testing::ElementsAreArray({0, 1, 2, 3}));
//     EXPECT_THAT(buffer2, ::testing::ElementsAreArray({1, 2, 2, 1}));

//     transceiver_.receiveData(std::vector<uint8_t>{1, 9, 2, 44});
//     EXPECT_THAT(buffer1, ::testing::ElementsAreArray({0, 1, 2, 3}));
//     EXPECT_THAT(buffer2, ::testing::ElementsAreArray({1, 2, 2, 1, 1, 9, 2, 44}));
// }

// TEST_F(MessageBrokerShould, publishMessages)
// {
//     MessageBroker sut(logger_factory_, transceiver_);
//     ExampleMessage message1(std::vector<uint8_t>{0, 1, 2, 3});
//     ExampleMessage message2(std::vector<uint8_t>{5, 6, 7, 8});

//     bool message_1_sent = false;
//     bool message_2_sent = false;
//     sut.publish(message1, [&message_1_sent]{ message_1_sent = true; }, []{});
//     transceiver_.notify_success_all();
//     EXPECT_TRUE(message_1_sent);
//     EXPECT_FALSE(message_2_sent);
//     sut.publish(message2, [&message_2_sent]{ message_2_sent = true; }, []{});
//     transceiver_.notify_success_all();
//     EXPECT_TRUE(message_1_sent);
//     EXPECT_TRUE(message_2_sent);
// }

// } // namespace msmp
