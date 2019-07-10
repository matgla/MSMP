#include <chrono>

#include "msmp/default_time_provider.hpp"

namespace msmp
{

std::chrono::milliseconds DefaultTimeProvider::milliseconds() const
{
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration);
}

} // namespace msmp
