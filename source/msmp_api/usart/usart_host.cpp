#include "tcp/tcp_host.hpp"

#include "tcp/connection.hpp"

#include "msmp_tcp/tcp_host.hpp"

#include <hal/usart.hpp>

namespace msmp_api
{

hal::interfaces::UsartInterface& getUsartByName(const std::string& device)
{
    if (device == "Usart1")
    {
        return hal::interfaces::UsartContainer.get<hal::interfaces::Usart0>();
    }
}

class UsartHostHolder
{
public:
    UsartHostHolder(const std::string& name, const std::string& device)
        : host_(name, getUsartByName(device))
    {

    }

    ~UsartHostHolder()
    {
    }

    void start()
    {
        host_.start();
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
    std::function<void()> callback_;
    hal::interfaces::Usart
    msmp::UsartHost host_;
};

UsartHost::UsartHost(const std::string& name, const std::string& device)
    : impl_(std::make_unique<UsartHostHolder>(name, host_port, peer_address, peer_port))
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

}
