#pragma once

#include <type_traits>

namespace msmp
{
namespace serializer
{

constexpr uint8_t __little_endian = 1;
constexpr uint8_t __big_endian = 2;

enum class Endian : uint8_t
{
    Big,
    Little,
#ifdef MSMP_HOST_BIG_ENDIAN
    Native = Big
#elif MSMP_HOST_LITTLE_ENDIAN
    Native = Little
#else
    Native = Little
#endif
};

} // namespace serializer
} // namespace msmp
