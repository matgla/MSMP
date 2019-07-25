#include "msmp_tcp/tcp_reader.hpp"

#include <functional>
#include <array>
#include <cstdint>
#include <memory>
#include <iostream>

#include "msmp/configuration/configuration.hpp"

namespace msmp
{

class Session : public std::enable_shared_from_this<Session>
{
public:
    using OnDataCallback = std::function<void(uint8_t)>;
    using OnDisconnectionCallback = std::function<void()>;
    Session(boost::asio::ip::tcp::socket socket, const OnDataCallback& on_data, const OnDisconnectionCallback& on_disconnection)
        : socket_(std::move(socket))
        , on_data_(on_data)
        , on_disconnection_(on_disconnection)
    {
    }

    void start()
    {
        doRead();
    }

private:
    void doRead()
    {
        auto self(shared_from_this());
        socket_.async_receive(boost::asio::buffer(data_), [this, self](boost::system::error_code ec, std::size_t length) {

            if (!ec)
            {
                std::size_t data_length = length < data_.size() ? length : data_.size();
                for (std::size_t i = 0; i < data_length; ++i)
                {
                    on_data_(data_[i]);
                }
                doRead();
            }
            else
            {
                if (on_disconnection_)
                {
                    on_disconnection_();
                }
            }
        });
    }

    std::array<uint8_t, 1024> data_;
    boost::asio::ip::tcp::socket socket_;
    OnDataCallback on_data_;
    OnDisconnectionCallback on_disconnection_;
};

TcpReader::TcpReader(boost::asio::io_service& io_service, uint16_t port, const OnDataCallback& on_data)
    : io_service_(io_service)
    , acceptor_(io_service_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    , socket_(io_service_)
    , on_data_(on_data)
    , connected_(false)
{
}

void TcpReader::start()
{
    doAccept();
}

void TcpReader::doOnConnection(const std::function<void()>& on_connection)
{
    on_connection_ = on_connection;
}

void TcpReader::doOnDisconnection(const std::function<void()>& callback)
{
    on_disconnection_ = callback;
}


void TcpReader::doAccept()
{
    acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
        if (!ec)
        {
            if (connected_)
            {
                return;
            }

            std::make_shared<Session>(std::move(socket_), on_data_, [this]{
                connected_ = false;
                if (on_disconnection_)
                {
                    on_disconnection_();
                }
            })->start();

            connected_ = true;

            if (on_connection_)
            {
                on_connection_();
            }
        }
    });
}

bool TcpReader::connected() const
{
    return connected_;
}

} // namespace msmp
