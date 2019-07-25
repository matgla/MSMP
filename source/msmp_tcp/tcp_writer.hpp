#pragma once

#include <string_view>
#include <functional>

#include <boost/asio.hpp>

#include "msmp/layers/physical/data_writer_base.hpp"

namespace msmp
{

class TcpWriter : public layers::physical::DataWriterBase
{
public:
    TcpWriter(boost::asio::io_service& io_service, const std::function<void()>& on_connected = []{},
        const std::function<void()>& on_disconnected = []{});

    void connect(std::string_view address, uint16_t port);
    void write(uint8_t byte) override;
    bool connected() const;
    void disconnect();
private:
    void performConnection(const boost::asio::ip::tcp::resolver::iterator& endpoints);

    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::socket socket_;
    bool connected_;
    std::function<void()> on_connected_;
    std::function<void()> on_disconnected_;
};

} // namespace msmp
