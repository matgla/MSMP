#pragma once

#include <string_view>

#include <eul/time/i_time_provider.hpp>
#include <eul/function.hpp>

#include "msmp/layers/session/connection.hpp"
#include "msmp/default_time_provider.hpp"
#include "msmp/host.hpp"

#include "msmp_tcp/tcp_writer.hpp"
#include "msmp_tcp/tcp_reader.hpp"


namespace msmp
{

class TcpHost
{
public:
    using CallbackType = Host::CallbackType;

    TcpHost(std::string_view name, uint16_t host_port,
        std::string_view peer_address, uint16_t peer_port);

    TcpHost(eul::time::i_time_provider& time_provider, std::string_view name,
        uint16_t host_port,
        std::string_view peer_address, uint16_t peer_port);

    void start();
    layers::session::Connection& getConnection();
    void onConnected(const CallbackType& callback);
private:
    boost::asio::io_service io_service_;
    TcpWriter tcp_writer_;
    TcpReader tcp_reader_;
    DefaultTimeProvider time_provider_;
    Host host_;
    std::string_view peer_address_;
    uint16_t peer_port_;

    bool connected_to_peer_;
    bool peer_is_connected_;
};

} // namespace msmp
