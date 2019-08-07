#pragma once

#include <cstdint>

#include <eul/function.hpp>

namespace msmp
{

class UsartReader
{
public:
    using OnDataCallback = eul::function<void(uint8_t), sizeof(void*)>;

    UsartReader(const OnDataCallback& on_data);

    void start();

private:
    OnDataCallback on_data_;
};

} // namespace msmp
