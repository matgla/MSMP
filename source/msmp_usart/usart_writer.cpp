#include "msmp_usart/usart_writer.hpp"

#include "msmp_usart/configuration.hpp"

namespace msmp
{

UsartWriter::UsartWriter(hal::interfaces::UsartInterface& usart)
    : usart_(usart)
{
}

void UsartWriter::start()
{
}

void UsartWriter::write(uint8_t byte)
{
    uint8_t data[1];
    data[0] = byte;
    usart_.write(data);
    on_success_.emit();
}

} // namespace msmp
