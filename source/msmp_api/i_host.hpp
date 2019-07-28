#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>

#include "i_connection.hpp"

namespace msmp_api
{

class IHost
{
public:
    using Callback = std::function<void()>;
    virtual ~IHost() = default;

    virtual void start() = 0;
    virtual void onConnected(const Callback& callback) = 0;
    virtual std::shared_ptr<IConnection> getConnection() = 0;
};

} // namespace msmp_api
