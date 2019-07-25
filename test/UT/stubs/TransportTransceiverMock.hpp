#pragma once

#include "msmp/layers/transport/transceiver/i_transport_transceiver.hpp"

namespace msmp
{
namespace test
{
namespace mocks
{

class TransportTransceiverMock : public layers::transport::transceiver::ITransportTransceiver
{
public:
    MOCK_METHOD1(respondNack, void(const Frame&));
    MOCK_METHOD1(respondAck, void(const Frame&));
    MOCK_METHOD1(onData, void(const CallbackType&));
    MOCK_METHOD1(send, void(const StreamType&));
    MOCK_METHOD3(send, void(const StreamType&, const TransmitterCallbackType&,
        const TransmitterCallbackType&));
    MOCK_METHOD0(start, void());
    MOCK_METHOD0(reset, void());

};

} // namespace mocks
} // namespace test
} // namespace msmp