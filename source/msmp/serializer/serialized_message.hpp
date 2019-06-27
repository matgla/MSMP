#pragma once

#include <memory>
#include <string_view>

#include <eul/container/static_vector.hpp>

#include "msmp/serializer/endian.hpp"
#include "msmp/serializer/serializers.hpp"

namespace msmp
{
namespace serializer
{

template <std::size_t Size = 0, Endian endian = Endian::Big>
struct SerializedMessage
{
public:
    using Serializer = Serializers<endian>;

    SerializedMessage()
    {
        buffer_.push_back(1); // MessageType -> User is default value
    }

    SerializedMessage(uint8_t message_type)
    {
        buffer_.push_back(message_type);
    }

    template <typename PreviousMessage, typename T>
    SerializedMessage(const PreviousMessage& previous, const T& data) : buffer_()
    {
        std::copy(previous.begin(), previous.end(), std::back_inserter(buffer_));
        std::copy(data.begin(), data.end(), std::back_inserter(buffer_));
    }

    constexpr auto compose_u8(uint8_t d)
    {
        return compose_impl(d);
    }

    constexpr auto compose_u16(uint16_t d)
    {
        return compose_impl(d);
    }

    constexpr auto compose_u32(uint32_t d)
    {
        return compose_impl(d);
    }

    constexpr auto compose_double(double d)
    {
        return compose_impl(d);
    }

    constexpr auto compose_float(float d)
    {
        return compose_impl(d);
    }

    constexpr auto compose_message(const auto& msg)
    {
        SerializedMessage<Size + msg.max_size()> new_msg(buffer_, msg);
        return new_msg;
    }

    template <std::size_t StringSize>
    auto compose_string(std::string_view str)
    {
        SerializedMessage<Size + StringSize> msg(buffer_, str);
        return msg.compose_u8(0);
    }

    template <std::size_t StringSize>
    auto compose_string(char const (&str)[StringSize])
    {
        SerializedMessage<Size + StringSize> msg(buffer_, std::string_view(str));
        return msg.compose_u8(0);
    }

    const auto build()
    {
        return buffer_;
    }

private:
    template <typename T>
    auto compose_impl(T t)
    {
        auto serialized = Serializer::serialize(t);
        SerializedMessage<Size + sizeof(T)> msg(buffer_, serialized);
        return msg;
    }

    eul::container::static_vector<uint8_t, Size> buffer_;
};

} // namespace serializer
} // namespace msmp
