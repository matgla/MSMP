#pragma once

#include <cstdint>

#include <eul/function.hpp>

#include <hal/usart.hpp>

namespace msmp
{

class UsartReader
{
public:
    using OnDataCallback = eul::function<void(uint8_t), sizeof(void*)>;

    UsartReader(hal::interfaces::UsartInterface& usart, const OnDataCallback& on_data);

    void start();

private:
    OnDataCallback on_data_;
    hal::interfaces::UsartInterface::OnDataSlot slot_;
    hal::interfaces::UsartInterface& usart_;
};

} // namespace msmp
