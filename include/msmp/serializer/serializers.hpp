#pragma once

#include <array>
#include <cstdint>

#include <type_traits>

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
        if constexpr (Endian == std::endian::big &&
                      is_same_endian<std::endian::native, std::endian::little>::value)
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
};

} // namespace serializer
} // namespace msmp