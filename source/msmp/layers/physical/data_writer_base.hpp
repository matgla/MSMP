#pragma once

#include "msmp/layers/physical/i_data_writer.hpp"

namespace msmp
{
namespace layers
{
namespace physical
{

class DataWriterBase : public IDataWriter
{
public:
    void doOnSuccess(OnSuccessSlot& slot) override;
    void doOnFailure(OnFailureSlot& slot) override;

protected:
    OnSuccessSignal on_success_;
    OnFailureSignal on_failure_;
};

} // namespace physical
} // namespace layers
} // namespace msmp
