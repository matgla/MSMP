#include <iostream>

#include <gsl/span>

#include <eul/logger/logger_stream_registry.hpp>

#include <msmp_tcp/tcp_host.hpp>
#include <msmp/broker/message_broker.hpp>

#include "message_a_handler.hpp"
#include "test/UT/stubs/StandardErrorStreamStub.hpp"

int main()
{
    stubs::StandardErrorStreamStub stream;
    eul::logger::logger_stream_registry::get().register_stream(stream);

    msmp::TcpHost host("TcpHostA", 1234, "localhost", 2345);
    msmp::broker::MessageBroker broker;

    broker.addConnection(host.getConnection());
    MessageAHandler handler_a;
    broker.addHandler(handler_a);

    host.onConnected([&broker] {
        auto msg_a = MessageA{
            .value = 177,
            .name = "TestingMessage"}.serialize();
        broker.publish(gsl::make_span(msg_a.begin(), msg_a.end()), []{
            std::cerr << "Message A successfuly published" << std::endl;
        }, [] {
            std::cerr << "Failure while publishing Message A" << std::endl;
        });
    });

    host.start();
}