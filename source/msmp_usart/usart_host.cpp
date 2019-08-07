#include "msmp_usart/usart_host.hpp"

namespace msmp
{

UsartHost::UsartHost(const std::string_view& name)
    : usart_reader_([this](uint8_t byte){host_.getDataLinkReceiver().receiveByte(byte);})
    , host_(time_provider_, usart_writer_, name)
{

}

void UsartHost::start()
{
    usart_reader_.start();
    usart_writer_.start();
}

layers::session::Connection& UsartHost::getConnection()
{
    return host_.getConnection();
}

void UsartHost::onConnected(const CallbackType& callback)
{
    host_.onConnected(callback);
}


}
