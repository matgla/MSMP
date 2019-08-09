#pragma once

#include <string_view>

#include <hal/usart.hpp>

#include "msmp/host.hpp"

#include "msmp_usart/usart_reader.hpp"
#include "msmp_usart/usart_writer.hpp"

#include "msmp/default_time_provider.hpp"


namespace msmp
{

class UsartHost
{
public:
    using CallbackType = Host::CallbackType;
    UsartHost(const std::string_view& name,  hal::interfaces::UsartInterface& usart);
    void start();

    layers::session::Connection& getConnection();
    void onConnected(const CallbackType& callback);

private:
    DefaultTimeProvider time_provider_;

    hal::interfaces::UsartInterface& usart_;
    UsartWriter usart_writer_;
    UsartReader usart_reader_;


    Host host_;
};

} // namespace msmp
