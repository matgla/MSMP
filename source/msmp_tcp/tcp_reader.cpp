#include "msmp_tcp/tcp_reader.hpp"

namespace msmp
{

TcpReader::TcpReader(boost::asio::io_service& io_service, std::string_view address, uint16_t port)
    : io_service_(io_service)
{
    (void)(address);
    (void)(port);
}

} // namespace msmp
