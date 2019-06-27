#include <gtest/gtest.h>

#include <eul/logger/logger_stream_registry.hpp>

#include "test/UT/stubs/StandardErrorStreamStub.hpp"

int main(int argc, char** argv)
{
    stubs::StandardErrorStreamStub stream;
    eul::logger::logger_stream_registry::get().register_stream(stream);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}