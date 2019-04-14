#include <cstdlib>
#include <iostream>
#include <string_view>

#include <gsl/span>

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

    template <typename T>
    static void write(const gsl::span<T>& data)
    {
        if (std::getenv("ENABLE_LOGGING"))
        {
            std::cerr << "{";
            for (typename gsl::span<T>::index_type i = 0; i < data.size() - 1; ++i)
            {
                std::cerr << static_cast<int>(data[i]) << ", ";
            }
            std::cerr << static_cast<int>(data[data.size() - 1]) << "}";
        }
    }
};

} // namespace stubs
