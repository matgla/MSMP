#pragma once

#include <eul/signals/signal.hpp>

namespace msmp
{
namespace layers
{
namespace physical
{

class IDataWriter
{
protected:
    using OnSuccessSignal = eul::signals::signal<void()>;
    using OnFailureSignal = eul::signals::signal<void()>;
public:
    using OnSuccessSlot = OnSuccessSignal::slot_t;
    using OnFailureSlot = OnFailureSignal::slot_t;

    virtual ~IDataWriter() = default;

    virtual void write(uint8_t byte) = 0;
    virtual void do_on_success(OnSuccessSlot& slot) = 0;
    virtual void do_on_failure(OnSuccessSlot& slot) = 0;
};

} // namespace physical
} // namespace layers
} // namespace msmp
