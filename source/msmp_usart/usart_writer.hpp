#pragma once

#include <cstdint>

#include <hal/usart.hpp>

#include "msmp/layers/physical/data_writer_base.hpp"

namespace msmp
{

class UsartWriter : public layers::physical::DataWriterBase
{
public:
    UsartWriter(hal::interfaces::UsartInterface& usart);
    void start();

    void write(uint8_t byte) override;

private:
    hal::interfaces::UsartInterface& usart_;
};

} // namespace msmp
