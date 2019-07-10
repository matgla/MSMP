#include <string_view>
#include <string>

#include "msmp_tcp/tcp_writer.hpp"

namespace msmp
{

TcpWriter::TcpWriter(boost::asio::io_service& io_service, std::string_view address, uint16_t port)
    : io_service_(io_service)
    , socket_(io_service_)
{
    auto endpoints = boost::asio::ip::tcp::resolver(io_service_).resolve({std::string(address), std::to_string(port)});
    boost::asio::async_connect(socket_, endpoints, [this](boost::system::error_code, boost::asio::ip::tcp::resolver::iterator)
    {

    });
}

void TcpWriter::write(uint8_t byte)
{
    uint8_t payload[1];
    payload[0] = byte;
    boost::asio::async_write(socket_, boost::asio::buffer(payload, 1),
        [this](boost::system::error_code, std::size_t)
        {

        });
}

} // namespace msmp
