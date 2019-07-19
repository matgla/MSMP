#include <iostream>

#include <gsl/span>

#include <eul/logger/logger_stream_registry.hpp>

#include <msmp_tcp/tcp_host.hpp>
#include <msmp/broker/message_broker.hpp>

#include "message_a_handler.hpp"
#include "test/UT/stubs/StandardErrorStreamStub.hpp"
#include "msmp/default_time_provider.hpp"

int main()
{
    stubs::StandardErrorStreamStub stream;
    eul::logger::logger_stream_registry::get().register_stream(stream);


    msmp::TcpHost host("TcpHostB", 1237, "localhost", 1236);
    msmp::DefaultTimeProvider time;
    eul::logger::logger_factory lf(time);
    msmp::broker::MessageBroker broker(lf);
    broker.addConnection(host.getConnection());
    MessageAHandler handler_a(broker);
    broker.addHandler(handler_a);

    host.onConnected([&broker] {
        std::cerr << "Peer connected!" << std::endl;
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