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
{
    tcp_reader_.doOnConnection([this]{
        tcp_writer_.connect(peer_address_, peer_port_);
    });
}


TcpHost::TcpHost(std::string_view name,
    uint16_t host_port,
    std::string_view peer_address, uint16_t peer_port)
    : tcp_writer_(io_service_)
    , tcp_reader_(io_service_, host_port, [this](uint8_t byte){host_.getDataLinkReceiver().receiveByte(byte);})
    , host_(time_provider_, tcp_writer_, name)
    , peer_address_(peer_address)
    , peer_port_(peer_port)
{
    tcp_reader_.doOnConnection([this]{
        tcp_writer_.connect(peer_address_, peer_port_);
    });
}

void TcpHost::start()
{
    tcp_writer_.connect(peer_address_, peer_port_);
    io_service_.post([this]{
        host_.connect();
        configuration::Configuration::execution_queue.run();
        configuration::Configuration::timer_manager.run();
    });
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
