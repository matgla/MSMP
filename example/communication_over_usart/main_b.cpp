#include <iostream>

#include <gsl/span>

#include <eul/logger/logger_stream_registry.hpp>

#include <msmp_usart/usart_host.hpp>
#include <msmp/broker/message_broker.hpp>
#include <msmp/configuration/configuration.hpp>

#include "message_a_handler.hpp"
#include "message_b_handler.hpp"
#include "test/UT/stubs/StandardErrorStreamStub.hpp"
#include "msmp/default_time_provider.hpp"


int main()
{
    stubs::StandardErrorStreamStub stream;
    eul::logger::logger_stream_registry::get().register_stream(stream);

    msmp::UsartHost host("HostB", hal::board::UsartContainer.get<hal::board::Usart1>());
    msmp::DefaultTimeProvider time;
    eul::logger::logger_factory lf(time);
    msmp::broker::MessageBroker broker(lf);

    broker.addConnection(host.getConnection());
    MessageAHandler handler_a(broker);
    MessageBHandler handler_b;
    broker.addHandler(handler_a);
    broker.addHandler(handler_b);

    host.onConnected([]{
        std::cout << "Peer connected!" << std::endl;
    });

    host.start();
}