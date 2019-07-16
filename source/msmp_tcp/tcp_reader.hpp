#pragma once

#include <functional>

#include <boost/asio.hpp>

namespace msmp
{

class TcpReader
{
public:
    using OnDataCallback = std::function<void(uint8_t)>;

    TcpReader(boost::asio::io_service& io_service, uint16_t port, const OnDataCallback& on_data);

    void doAccept();
    void doOnConnection(const std::function<void()>& on_connection);
    bool connected() const;
private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
    OnDataCallback on_data_;
    std::function<void()> on_connection_;
    bool connected_;
};

} // namespace msmp
