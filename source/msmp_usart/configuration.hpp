#pragma once

#include <hal/usart.hpp>

namespace msmp
{

class UsartConfiguration
{
public:
    constexpr static auto& UsartPort = hal::interfaces::Usart1;
};

} // namespace msmp
