#pragma once

#include <memory>

#include "i_host.hpp"

namespace msmp_api
{

class UsartHostHolder;

class UsartHost : public IHost
{
public:
    UsartHost(const std::string& name, const std::string& device);

    void start() override;
    void onConnected(const Callback& callback) override;
    std::shared_ptr<IConnection> getConnection() override;

private:
    std::shared_ptr<UsartHostHolder> impl_;
};

} // namespace msmp_api