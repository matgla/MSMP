#pragma once

#include <memory>

#include "i_host.hpp"

namespace msmp_api
{

class TcpHostHolder;

class TcpHost : public IHost
{
public:
    TcpHost(const std::string& name, uint16_t host_port, const std::string& peer_address, uint16_t peer_port);

    void start() override;
    void onConnected(const Callback& callback) override;
    std::shared_ptr<IConnection> getConnection() override;

private:
    std::shared_ptr<TcpHostHolder> impl_;
};

} // namespace msmp_api