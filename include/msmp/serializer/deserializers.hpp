#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <type_traits>

#include <gsl/span>

#include <eul/container/static_vector.hpp>

#include "msmp/serializer/endian_type_traits.hpp"

namespace msmp
{
namespace serializer
{

template <std::endian Endian>
struct Deserializers
{
    template <typename T, typename E = std::enable_if_t<std::conjunction_v<std::is_fundamental<T>>>>
    constexpr static T deserialize(const gsl::span<const uint8_t>& data)
    {
        T deserialized  = 0;
        uint8_t* memory = reinterpret_cast<uint8_t*>(&deserialized);

        if constexpr ((Endian == std::endian::big &&
                       is_same_endian<std::endian::native, std::endian::little>::value) ||
                      (Endian == std::endian::little &&
                       is_same_endian<std::endian::native, std::endian::big>::value))
        {
            for (std::size_t i = 0; i < sizeof(T); ++i)
            {
                memory[i] = data[sizeof(T) - 1 - i];
            }
            return deserialized;
        }

        for (std::size_t i = 0; i < sizeof(T); ++i)
        {
            memory[i] = data[i];
        }
        return deserialized;
    }
};

} // namespace serializer
} // namespace msmp