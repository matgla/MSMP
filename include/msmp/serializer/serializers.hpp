#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <type_traits>

#include <eul/container/static_vector.hpp>

namespace msmp
{
namespace serializer
{

template <std::endian a, std::endian b>
struct is_same_endian
{
    constexpr static bool value = a == b;
};

template <std::endian Endian>
struct Serializers
{
    template <typename T, typename E = std::enable_if_t<std::conjunction_v<std::is_fundamental<T>>>>
    constexpr static std::array<uint8_t, sizeof(T)> serialize(const T& data)
    {
        if constexpr ((Endian == std::endian::big &&
                       is_same_endian<std::endian::native, std::endian::little>::value) ||
                      (Endian == std::endian::little &&
                       is_same_endian<std::endian::native, std::endian::big>::value))
        {
            std::array<uint8_t, sizeof(T)> serialized{};
            for (std::size_t i = 0; i < serialized.size(); ++i)
            {
                const uint8_t* memory = reinterpret_cast<const uint8_t*>(&data);
                serialized[i]         = memory[sizeof(T) - 1 - i];
            }
            return serialized;
        }

        std::array<uint8_t, sizeof(T)> serialized{};
        for (std::size_t i = 0; i < serialized.size(); ++i)
        {
            const uint8_t* memory = reinterpret_cast<const uint8_t*>(&data);
            serialized[i]         = memory[i];
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