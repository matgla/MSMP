#pragma once

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

} // namespace serializer
} // namespace msmp
