#include <string_view>
#include <string>

#include "msmp_tcp/tcp_writer.hpp"
#include "msmp/configuration/configuration.hpp"

namespace msmp
{

TcpWriter::TcpWriter(boost::asio::io_service& io_service,
    const std::function<void()>& on_connected,
    const std::function<void()>& on_disconnected)
    : io_service_(io_service)
    , socket_(io_service_)
    , connected_(false)
    , on_connected_(on_connected)
    , on_disconnected_(on_disconnected)
{
}

void TcpWriter::connect(std::string_view address, uint16_t port)
{
    auto endpoints = boost::asio::ip::tcp::resolver(io_service_).resolve({std::string(address), std::to_string(port)});
    performConnection(endpoints);
}

void TcpWriter::disconnect()
{
    connected_ = false;
}


void TcpWriter::performConnection(const boost::asio::ip::tcp::resolver::iterator& endpoints)
{
    if (!connected_)
    {
        boost::asio::async_connect(socket_, endpoints, [this, endpoints](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator)
        {
            if (ec)
            {
                if (connected_)
                {
                    connected_ = false;
                    if (on_disconnected_)
                    {
                        on_disconnected_();
                    }
                }
                io_service_.post([this, endpoints] {
                    performConnection(endpoints);
                });

                return;
            }
            connected_ = true;
            if (on_connected_)
            {
                on_connected_();
            }
        });
    }
}

void TcpWriter::write(uint8_t byte)
{
    uint8_t payload[1];
    payload[0] = byte;
    boost::asio::async_write(socket_, boost::asio::buffer(payload, 1),
        [this](boost::system::error_code ec, std::size_t)
        {
            if (ec)
            {
                on_failure_.emit();
                return;
            }
            on_success_.emit();
            configuration::Configuration::execution_queue.run();
        });
}

bool TcpWriter::connected() const
{
    return connected_;
}

} // namespace msmp
