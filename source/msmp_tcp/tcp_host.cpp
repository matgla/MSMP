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
    , sm_{TcpHostSm{[this] {
        host_.connect();
    }}}
    , sm_data_(sm_)
{
    tcp_reader_.doOnConnection([this] {
        sm_.process_event(ConnectionReceived{});
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
    tcp_writer_.connect(peer_address_, peer_port_, [this] {
        sm_.process_event(ConnectionEstablished{});
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
