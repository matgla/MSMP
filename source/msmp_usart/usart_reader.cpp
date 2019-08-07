#include "msmp_usart/usart_reader.hpp"

#include "msmp_usart/configuration.hpp"

namespace msmp
{

UsartReader::UsartReader(const OnDataCallback& on_data)
    : on_data_(on_data)
{

}

void UsartReader::start()
{
    UsartConfiguration::UsartPort.init(115200);
    UsartConfiguration::UsartPort.onData(on_data_);
}

}