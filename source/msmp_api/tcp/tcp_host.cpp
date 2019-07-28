#include "tcp/tcp_host.hpp"

#include "tcp/connection.hpp"

#include "msmp_tcp/tcp_host.hpp"

namespace msmp_api
{

class TcpHostHolder
{
public:
    TcpHostHolder(const std::string& name, uint16_t host_port,
        const std::string& peer_address, uint16_t peer_port)
        : host_(name, host_port, peer_address, peer_port)
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
    msmp::TcpHost host_;
};

TcpHost::TcpHost(const std::string& name, uint16_t host_port, const std::string& peer_address, uint16_t peer_port)
    : impl_(std::make_unique<TcpHostHolder>(name, host_port, peer_address, peer_port))
{

}

void TcpHost::start()
{
    impl_->start();
}

void TcpHost::onConnected(const Callback& callback)
{
    impl_->onConnected(callback);
}

std::shared_ptr<IConnection> TcpHost::getConnection()
{
    return impl_->getConnection();
}

}
