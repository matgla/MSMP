#include "msmp/host.cpp"

#include "msmp_tcp/tcp_host.hpp"

namespace msmp
{

TcpHost::TcpHost(eul::time::i_time_provider& time_provider, std::string_view name,
    std::string_view host_address, uint16_t host_port,
    std::string_view peer_address, uint16_t peer_port)
    : Host(time_provider, tcp_writer_, name)
    , tcp_writer_(io_service_, peer_address, peer_port)
    , tcp_reader_(io_service_, host_address, host_port)
{
}


TcpHost::TcpHost(std::string_view name,
    std::string_view host_address, uint16_t host_port,
    std::string_view peer_address, uint16_t peer_port)
    : Host(time_provider_, tcp_writer_, name)
    , tcp_writer_(io_service_, peer_address, peer_port)
    , tcp_reader_(io_service_, host_address, host_port)
{

}

} // namespace msmp
