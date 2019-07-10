#pragma once

#include <string_view>

#include <eul/time/i_time_provider.hpp>

#include "msmp/default_time_provider.hpp"
#include "msmp/host.hpp"

#include "msmp_tcp/tcp_writer.hpp"
#include "msmp_tcp/tcp_reader.hpp"


namespace msmp
{

class TcpHost : public Host
{
public:
    TcpHost(std::string_view name,
        std::string_view host_address, uint16_t host_port,
        std::string_view peer_address, uint16_t peer_port);

    TcpHost(eul::time::i_time_provider& time_provider, std::string_view name,
        std::string_view host_address, uint16_t host_port,
        std::string_view peer_address, uint16_t peer_port);

private:
    TcpWriter tcp_writer_;
    TcpReader tcp_reader_;
    boost::asio::io_service io_service_;
    DefaultTimeProvider time_provider_;
};

} // namespace msmp
