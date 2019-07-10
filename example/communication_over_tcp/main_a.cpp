#include <iostream>

#include <msmp_tcp/tcp_host.hpp>

int main()
{
    msmp::TcpHost host("TcpHostA", "localhost", 8001, "localhost", 8002);
    std::cerr << "Peer A is starting" << std::endl;
}