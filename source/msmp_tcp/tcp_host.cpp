#include "msmp/host.cpp"

#include "msmp_tcp/tcp_host.hpp"

#include <iostream>

namespace msmp
{

TcpHost::TcpHost(eul::time::i_time_provider& time_provider, std::string_view name,
    uint16_t host_port,
    std::string_view peer_address, uint16_t peer_port)
    : tcp_writer_(io_service_, [this] {
        sm_.process_event(ConnectionEstablished{});
    }, [this] {
        sm_.process_event(PeerDisconnected{});
        sm_.process_event(Disconnect{});
        host_.peerDisconnected();
        tcp_writer_.disconnect();
        tcp_writer_.connect(peer_address_, peer_port_);
        tcp_reader_.start();
    })
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

    tcp_reader_.doOnDisconnection([this] {
        sm_.process_event(Disconnect{});
        sm_.process_event(PeerDisconnected{});
        host_.peerDisconnected();
        tcp_writer_.disconnect();
        tcp_writer_.connect(peer_address_, peer_port_);
        tcp_reader_.start();
    });
    std::cerr << "this-> 0x" << std::hex << this << std::endl;
}


TcpHost::TcpHost(std::string_view name,
    uint16_t host_port,
    std::string_view peer_address, uint16_t peer_port)
    : TcpHost(time_provider_, name, host_port, peer_address, peer_port)
{
    std::cerr << "Created: " << name << ". Connecting to " << peer_address << ":" << peer_port << std::endl;
}

TcpHost::~TcpHost()
{
    host_.disconnect();
}


void TcpHost::start()
{
    std::cerr << "this-> 0x" << std::hex << this << std::endl;
    std::cerr << "Connecting to: " << peer_address_ << ":" << peer_port_ << std::endl;

    tcp_writer_.connect(peer_address_, peer_port_);
    tcp_reader_.start();

    // Construct a signal set registered for process termination.
    boost::asio::signal_set signals(io_service_, SIGINT, SIGTERM);
    signals.async_wait(
        std::bind(&boost::asio::io_service::stop, &io_service_));
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
