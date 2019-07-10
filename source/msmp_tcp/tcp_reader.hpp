#pragma once

#include <boost/asio.hpp>

namespace msmp
{

class TcpReader
{
public:
    TcpReader(boost::asio::io_service& io_service, std::string_view address, uint16_t port);

private:
    boost::asio::io_service& io_service_;
};

} // namespace msmp
