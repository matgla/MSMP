#pragma once

#include <cstdint>
#include <cstring>
#include <string_view>

#include <gsl/span>

#include <eul/utils/string.hpp>

#include "msmp/serializer/deserializers.hpp"

namespace msmp
{
namespace serializer
{
namespace detail
{
template <std::size_t position = 0, Endian endian = Endian::Big>
class MessageDeserializer
{
public:
    using Deserializer = Deserializers<endian>;
    using DataType = gsl::span<const uint8_t>;
    MessageDeserializer(const DataType& data)
        : buffer_(data)
        , position_(position)
    {
    }

    template <typename T>
    void decompose(T& data)
    {
        data = decompose_impl<T>();
    }

    template <std::size_t Size>
    void decompose(char (&data)[Size])
    {
        auto deserialized_string = decompose_string();
        const std::size_t length = deserialized_string.length() + 1 < Size ? deserialized_string.length() : Size - 1;
        std::copy(deserialized_string.begin(), deserialized_string.begin() + length, data);
        data[length + 1] = 0;
    }

    uint8_t decompose_u8()
    {
        return decompose_impl<uint8_t>();
    }

    uint16_t decompose_u16()
    {
        return decompose_impl<uint16_t>();
    }

    uint32_t decompose_u32()
    {
        return decompose_impl<uint32_t>();
    }

    float decompose_float()
    {
        return decompose_impl<float>();
    }

    std::string_view decompose_string()
    {
        const std::string_view str(reinterpret_cast<const char*>(buffer_.data()) + position_, eul::utils::strlen(buffer_.subspan(position_, buffer_.size() - position_)));
        position_ += str.size() + 1;
        return str;
    }

    void drop_u8()
    {
        position_ += 1;
    }

    void drop_u16()
    {
        position_ += 2;
    }

    void drop_u32()
    {
        position_ += 4;
    }
private:
    template <typename T>
    T decompose_impl()
    {
        const auto data = Deserializer::template deserialize<T>(buffer_.subspan(position_, sizeof(T)));
        position_ += sizeof(T);
        return data;
    }

    DataType buffer_;
    std::size_t position_;
};
} // namespace detail

template<Endian endian = Endian::Big>
using UserMessageDeserializer = detail::MessageDeserializer<1, endian>;

template<Endian endian = Endian::Big>
using ProtocolMessageDeserializer = detail::MessageDeserializer<1, endian>;

template<Endian endian = Endian::Big>
using RawMessageDeserializer = detail::MessageDeserializer<0, endian>;

template<Endian endian = Endian::Big>
using DataDeserializer = RawMessageDeserializer<endian>;


} // namespace serializer
} // namespace msmp
