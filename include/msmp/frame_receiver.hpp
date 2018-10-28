#pragma once

#include <eul/function.hpp>
#include <eul/mpl/mixin/access.hpp>
#include <eul/type_traits.hpp>

#include "types.hpp"

namespace msmp
{

template <typename T>
struct is_receiver : std::false_type
{
};

template <typename T>
concept bool is_receiver_t = requires(T a)
{
    // clang-format off
    { a.receive(DataStream{})} -> void;
    { a.set_writer(DataStream{})} -> void;
    // clang-format on
};

template <is_receiver T>
struct is_receiver<T> : std::true_type
{
};

template <typename T>
struct is_buffer : std::false_type
{
};

template <typename T>
concept bool is_buffer_t = requires(T a)
{
    // clang-format off
    { a.write(DataStream{})} -> void;
    // clang-format on
};

template <is_buffer_t T>
struct is_buffer<T> : std::true_type
{
};

template <typename ObjectType>
class IFrameReceiver
{
public:
    IFrameReceiver()
    {
        auto data = eul::mpl::mixin::access<ObjectType>(this);
        data.for_each(eul::mpl::mixin::ability<is_receiver>{},
                      [this, &payload](auto& receiver) {
                          receiver.set_writer([this](const DataStream& data) { write_to_buffer(data); });
                      });
    }


    void receive_data(const DataStream& payload)
    {
        auto data = eul::mpl::mixin::access<ObjectType>(this);
        data.for_each(eul::mpl::mixin::ability<is_receiver>{},
                      [this, &payload](auto& receiver) {
                          receiver.receive(payload);
                      });
    }

protected:
    void write_to_buffer(const DataStream& data)
    {
        auto data = eul::mpl::mixin::access<ObjectType>(this);
        data.for_each(eul::mpl::mixin::ability<is_buffer>{}, [&data](auto& buffer) {
            buffer.write(data);
        })
    }
};

class FrameReceiver
{
public:
    using WriteToBufferCallback = eul::function<eul::pointer_size::size, void(const DataStream& data)>;
    FrameReceiver();
    void receive(const DataStream& payload);
    void set_writer(const WriteToBufferCallback& writer);

protected:
    enum class States
    {
        Uninitialized,
        WaitForStartByte,
        ReadHeader,
        VerifyHeader,
        ReadFramePayload,
        VerifyPayload,
        SendAcknowledge,
        Failed
    };

    States state_;
    WriteToBufferCallback write_to_buffer_;
};

} // namespace msmp
