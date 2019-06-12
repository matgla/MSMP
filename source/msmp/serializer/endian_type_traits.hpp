#pragma once

#include <type_traits>

#include "msmp/serializer/endian.hpp"

namespace msmp
{
namespace serializer
{

template <Endian a, Endian b>
struct is_same_endian
{
    constexpr static bool value = a == b;
};

} // namespace serializer
} // namespace msmp
