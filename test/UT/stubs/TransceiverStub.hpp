#pragma once

#include <algorithm>
#include <vector>

#include <gsl/span>

#include <eul/function.hpp>

#include "msmp/types.hpp"

namespace test
{
namespace stubs
{

class TransceiverStub
{
public:
    using StreamType = msmp::StreamType;
    using CallbackType = eul::function<void(const StreamType&), sizeof(void*)>;
    using TransportCallbackType = eul::function<void(), sizeof(void*)>;
    using TransmitterCallbackType = CallbackType;

    struct Transmission
    {
    public:
        Transmission(const StreamType& payload, const TransportCallbackType& on_success, const TransportCallbackType& on_failure)
            : on_success_(on_success)
            , on_failure_(on_failure)
        {
            std::copy(payload.begin(), payload.end(), std::back_inserter(payload_));
        }

        void notify_success()
        {
            if (on_success_)
            {
                on_success_();
            }
        }

        void notify_failure()
        {
            if (on_failure_)
            {
                on_failure_();
            }
        }

    private:
        std::vector<uint8_t> payload_;
        TransportCallbackType on_success_;
        TransportCallbackType on_failure_;
    };

    void on_data(const CallbackType& callback)
    {
        on_data_ = callback;
    }

    void receive_data(const gsl::span<const uint8_t>& payload)
    {
        if (on_data_)
        {
            on_data_(payload);
        }
    }

    void send(const StreamType& payload, const TransportCallbackType& on_success, const TransportCallbackType& on_failure)
    {
        transmissions_.push_back(Transmission(payload, on_success, on_failure));
    }

    const std::vector<Transmission>& get_transmissions() const
    {
        return transmissions_;
    }

    std::vector<Transmission>& get_transmissions()
    {
        return transmissions_;
    }

    void notify_success_all()
    {
        for (auto& transmission : transmissions_)
        {
            transmission.notify_success();
        }
        transmissions_.clear();
    }

private:
    std::vector<Transmission> transmissions_;
    CallbackType on_data_;
};

} // namespace stubs
} // namespace test
