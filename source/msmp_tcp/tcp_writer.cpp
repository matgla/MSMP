#include <string_view>
#include <string>
#include <iostream>

#include "msmp_tcp/tcp_writer.hpp"

namespace msmp
{

TcpWriter::TcpWriter(boost::asio::io_service& io_service)
    : io_service_(io_service)
    , socket_(io_service_)
    , connected_(false)
{
}

void TcpWriter::connect(std::string_view address, uint16_t port)
{
    auto endpoints = boost::asio::ip::tcp::resolver(io_service_).resolve({std::string(address), std::to_string(port)});
    performConnection(endpoints);
}


void TcpWriter::performConnection(const boost::asio::ip::tcp::resolver::iterator& endpoints)
{
    if (!connected_)
    {
        boost::asio::async_connect(socket_, endpoints, [this, endpoints](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator)
        {
            if (ec)
            {
                std::cerr << "Error received" << std::endl;

                    io_service_.post([this, endpoints] {
                        performConnection(endpoints);
                    });

                return;
            }
            std::cerr << "Connected to server" << std::endl;
            connected_ = true;
        });
    }
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
