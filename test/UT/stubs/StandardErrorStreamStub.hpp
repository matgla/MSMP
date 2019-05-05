#include <cstdlib>
#include <iostream>
#include <string_view>

#include <gsl/span>

#include "eul/logger/logger_stream.hpp"

namespace stubs
{

struct StandardErrorStreamStub : public eul::logger::logger_stream
{
    void write(const std::string_view& data) override
    {
        if (std::getenv("ENABLE_LOGGING"))
        {
            std::cerr << data;
        }
    }
};

} // namespace stubs
