#include "bsp/onewire.hpp"

#include "utils.hpp"

namespace bsp
{

template <>
OneWire<OneWires::OneWire1>::OneWire()
{
    // TODO: implement
}

template <>
void OneWire<OneWires::OneWire1>::setAsInput()
{
    // TODO: implement
}

template <>
void OneWire<OneWires::OneWire1>::setAsOutput()
{
    // TODO: implement
}

template <>
BusState OneWire<OneWires::OneWire1>::getState()
{
    // TODO: implement
    return BusState::Low;
}

template <>
void OneWire<OneWires::OneWire1>::setState(BusState state)
{
    UNUSED(state);
    // TODO: implement
}

} // namespace bsp
