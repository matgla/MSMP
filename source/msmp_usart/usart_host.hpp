#pragma once

#include <string_view>

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
    UsartHost(const std::string_view& name);
    void start();

    layers::session::Connection& getConnection();
    void onConnected(const CallbackType& callback);

private:
    DefaultTimeProvider time_provider_;

    UsartWriter usart_writer_;
    UsartReader usart_reader_;

    Host host_;
};

} // namespace msmp
