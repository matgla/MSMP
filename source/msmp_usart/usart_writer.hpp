#pragma once

#include <cstdint>

#include "msmp/layers/physical/data_writer_base.hpp"

namespace msmp
{

class UsartWriter : public layers::physical::DataWriterBase
{
public:
    void start();

    void write(uint8_t byte) override;
};

} // namespace msmp
