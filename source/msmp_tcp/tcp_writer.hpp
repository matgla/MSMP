#pragma once

#include <string_view>

#include <boost/asio.hpp>

#include "msmp/layers/physical/data_writer_base.hpp"

namespace msmp
{

class TcpWriter : public layers::physical::DataWriterBase
{
public:
    TcpWriter(boost::asio::io_service& io_service);

    void connect(std::string_view address, uint16_t port);
    void write(uint8_t byte) override;

private:
    void performConnection(const boost::asio::ip::tcp::resolver::iterator& endpoints);

    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::socket socket_;
    bool connected_;
};

} // namespace msmp
