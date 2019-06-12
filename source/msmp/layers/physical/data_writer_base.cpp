#include "msmp/layers/physical/data_writer_base.hpp"

namespace msmp
{
namespace layers
{
namespace physical
{

void DataWriterBase::do_on_success(OnSuccessSlot& slot)
{
    on_success_.connect(slot);
}

void DataWriterBase::do_on_failure(OnSuccessSlot& slot)
{
    on_success_.connect(slot);
}

} // namespace physical
} // namespace layers
} // namespace msmp
