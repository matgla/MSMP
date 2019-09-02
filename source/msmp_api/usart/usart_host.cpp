#include <cstdlib>
#include <iostream>
#include <string_view>

#include <eul/logger/logger_stream.hpp>
#include <eul/logger/logger_stream_registry.hpp>

#include "msmp_api/usart/usart_host.hpp"

#include "tcp/connection.hpp"

#include "msmp_usart/usart_host.hpp"

#include <hal/usart.hpp>

namespace msmp_api
{

struct StandardErrorLogger : public eul::logger::logger_stream
{
    void write(const std::string_view& data) override
    {
        if (std::getenv("ENABLE_LOGGING"))
        {
            std::cerr << data;
        }
    }
};

hal::interfaces::UsartInterface& getUsartByName(const std::string& device)
{
    if (device == "Usart0")
    {
        return hal::board::UsartContainer.get<hal::board::Usart0>();
    }
    else if (device == "Usart1")
    {
        return hal::board::UsartContainer.get<hal::board::Usart1>();
    }
    return hal::board::UsartContainer.get<hal::board::Usart0>();
}

class UsartHostHolder
{
public:
    UsartHostHolder(const std::string& name, const std::string& device)
        : usart_(getUsartByName(device))
        , name_(name)
        , host_(name_, usart_)
    {
        static bool was_logger_initialized = false;
        if (!was_logger_initialized)
        {
            eul::logger::logger_stream_registry::get().register_stream(stream_);
            was_logger_initialized = true;
        }
    }

    ~UsartHostHolder()
    {
    }

    void start()
    {
        std::cerr << "Initialziing" << std::endl;
        usart_.init(9600);
        std::cerr << "Thread spawned" << std::endl;

        host_.start();
        std::cerr << "Connection enstablished" << std::endl;
    }

    void onConnected(const std::function<void()>& callback)
    {
        callback_ = callback;
        host_.onConnected([this]{
            callback_();
        });
    }

    std::unique_ptr<IConnection> getConnection()
    {
        return std::make_unique<Connection>(host_.getConnection());
    }

private:
    StandardErrorLogger stream_;
    std::function<void()> callback_;
    hal::interfaces::UsartInterface& usart_;
    std::string name_;
    msmp::UsartHost host_;
};

UsartHost::UsartHost(const std::string& name, const std::string& device)
    : impl_(std::make_unique<UsartHostHolder>(name, device))
{

}

void UsartHost::start()
{
    impl_->start();
}

void UsartHost::onConnected(const Callback& callback)
{
    impl_->onConnected(callback);
}

std::shared_ptr<IConnection> UsartHost::getConnection()
{
    return impl_->getConnection();
}

} // namespace msmp_api
