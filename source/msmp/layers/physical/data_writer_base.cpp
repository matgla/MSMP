#include "msmp/layers/physical/data_writer_base.hpp"

namespace msmp
{
namespace layers
{
namespace physical
{

void DataWriterBase::doOnSuccess(OnSuccessSlot& slot)
{
    on_success_.connect(slot);
}

void DataWriterBase::doOnFailure(OnFailureSlot& slot)
{
    on_failure_.connect(slot);
}

} // namespace physical
} // namespace layers
} // namespace msmp
