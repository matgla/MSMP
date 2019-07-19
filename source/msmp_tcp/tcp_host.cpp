#include "msmp/host.cpp"

#include "msmp_tcp/tcp_host.hpp"

#include <iostream>

namespace msmp
{

TcpHost::TcpHost(eul::time::i_time_provider& time_provider, std::string_view name,
    uint16_t host_port,
    std::string_view peer_address, uint16_t peer_port)
    : tcp_writer_(io_service_)
    , tcp_reader_(io_service_, host_port, [this](uint8_t byte){host_.getDataLinkReceiver().receiveByte(byte);})
    , host_(time_provider, tcp_writer_, name)
    , peer_address_(peer_address)
    , peer_port_(peer_port)
    , host_connected_(false)
{
    tcp_reader_.doOnConnection([this]{
        if (!tcp_writer_.connected())
        {
            std::cerr << "Writer not connected" << std::endl;
            tcp_writer_.connect(peer_address_, peer_port_, [this]{
                if (tcp_reader_.connected() && tcp_writer_.connected())
                {
                    std::cerr << "Writer and reader connected, starting host" << std::endl;
                    host_connected_ = true;
                    if (!host_connected_)
                    {
                        host_.connect();
                    }
                }
            });
        }

        if (tcp_reader_.connected() && tcp_writer_.connected())
        {
            std::cerr << "Writer and reader connected, starting host from reader" << std::endl;

            host_connected_ = true;
            if (!host_connected_)
            {
                host_.connect();
            }
        }
    });
}


TcpHost::TcpHost(std::string_view name,
    uint16_t host_port,
    std::string_view peer_address, uint16_t peer_port)
    : TcpHost(time_provider_, name, host_port, peer_address, peer_port)
{

}

void TcpHost::start()
{
    tcp_writer_.connect(peer_address_, peer_port_, [this]{
        if (tcp_reader_.connected() && tcp_writer_.connected())
        {
            std::cerr << "Writer and reader connected, starting host from start" << std::endl;

            host_connected_ = true;
            if (!host_connected_)
            {
                host_.connect();
            }
        }
    });
    tcp_reader_.start();
    io_service_.run();
}

layers::session::Connection& TcpHost::getConnection()
{
    return host_.getConnection();
}

void TcpHost::onConnected(const CallbackType& callback)
{
    host_.onConnected(callback);
}

} // namespace msmp
