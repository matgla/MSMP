#include "msmp_usart/usart_reader.hpp"

#include "msmp_usart/configuration.hpp"

namespace msmp
{

UsartReader::UsartReader(hal::interfaces::UsartInterface& usart, const OnDataCallback& on_data)
    : on_data_(on_data)
    , usart_(usart)
{
    slot_ = [this] (const hal::interfaces::UsartInterface::StreamType& data) {
        for (const auto byte : data)
        {
            on_data_(byte);
        }
    };
}

void UsartReader::start()
{
    usart_.init(115200);
    usart_.onData(slot_);
}

}