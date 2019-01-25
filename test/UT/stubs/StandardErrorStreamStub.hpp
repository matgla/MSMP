#include <cstdlib>
#include <iostream>
#include <string_view>

namespace stubs
{

struct StandardErrorStreamStub
{
    static void write(const std::string_view& data)
    {
        if (std::getenv("ENABLE_LOGGING"))
        {
            std::cerr << data;
        }
    }
};

} // namespace stubs
