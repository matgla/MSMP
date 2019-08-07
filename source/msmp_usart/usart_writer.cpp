#include "msmp_usart/usart_writer.hpp"

#include "msmp_usart/configuration.hpp"

namespace msmp
{

void UsartWriter::start()
{
    UsartConfiguration::UsartPort.init(115200);
}

void UsartWriter::write(uint8_t byte)
{
    UsartConfiguration::UsartPort.write(byte);
}

} // namespace msmp
