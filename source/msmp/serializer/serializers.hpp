#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <type_traits>

#include <eul/container/static_vector.hpp>

#include "msmp/serializer/endian.hpp"
#include "msmp/serializer/endian_type_traits.hpp"

namespace msmp
{
namespace serializer
{

template <Endian endian>
struct Serializers
{
    template <typename T, typename E = std::enable_if_t<std::conjunction_v<std::is_fundamental<T>>>>
    constexpr static std::array<uint8_t, sizeof(T)> serialize(const T& data)
    {
        std::array<uint8_t, sizeof(T)> serialized{};
        const uint8_t* memory = reinterpret_cast<const uint8_t*>(&data);

        if constexpr ((endian == Endian::Big &&
                       is_same_endian<Endian::Native, Endian::Little>::value) ||
                      (endian == Endian::Little &&
                       is_same_endian<Endian::Native, Endian::Big>::value))
        {
            for (std::size_t i = 0; i < serialized.size(); ++i)
            {
                serialized[i] = memory[sizeof(T) - 1 - i];
            }
            return serialized;
        }

        for (std::size_t i = 0; i < serialized.size(); ++i)
        {
            serialized[i] = memory[i];
        }
        return serialized;
    }

    template <std::size_t N>
    constexpr static auto serialize(const std::string_view& str)
    {
        eul::container::static_vector<uint8_t, N + 1> serialized;
        const std::size_t elements_to_copy = str.length() < N ? str.length() : N;

        for (std::size_t i = 0; i < elements_to_copy; ++i)
        {
            serialized[i] = str[i];
        }

        serialized.push_back('\0');

        return serialized;
    }

    template <std::size_t N>
    constexpr static auto serialize(const char (&str)[N])
    {
        return serialize<N>(std::string_view{str});
    }


    constexpr static std::size_t length(const char* str)
    {
        return length_impl(str, 0);
    }

    template <std::size_t N>
    constexpr static std::size_t length(const char (&str)[N])
    {
        return N;
    }

private:
    constexpr static std::size_t length_impl(const char* str, std::size_t index)
    {
        return str[index] == 0 ? index : length_impl(str, index + 1);
    }
};

} // namespace serializer
} // namespace msmp