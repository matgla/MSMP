#include "msmp/frame_receiver.hpp"

namespace msmp
{

FrameReceiver::FrameReceiver()
    : state_(FrameReceiver::States::Uninitialized)
{
}

} // namespace msmp