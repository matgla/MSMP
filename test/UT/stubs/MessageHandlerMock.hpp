#pragma once

#include "msmp/broker/message_handler.hpp"

#include <gmock/gmock.h>

namespace msmp
{
namespace broker
{

class MessageHandlerMock : public MessageHandler
{
public:
    MOCK_CONST_METHOD2(match, bool(uint8_t, const StreamType&));
    MOCK_METHOD1(handle, void(const StreamType&));
};

} // namespace broker
} // namespace msmp
