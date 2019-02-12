#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <eul/logger/logger_factory.hpp>
#include <eul/mpl/mixin/access.hpp>
#include <eul/mpl/mixin/data.hpp>
#include <eul/mpl/mixin/interface.hpp>
#include <eul/mpl/mixin/object.hpp>
#include <eul/mpl/types/bind_type.hpp>

#include "msmp/data_link_transmitter.hpp"

#include "test/UT/stubs/StandardErrorStreamStub.hpp"
#include "test/UT/stubs/TimeStub.hpp"
#include "test/UT/stubs/TimerManagerStub.hpp"

using namespace eul::mpl::mixin;

namespace msmp
{

template <typename T>
struct A
{
    A(T t) : t(t)
    {
    }
    int a()
    {
        return 19;
    }

    T t;
};

class DataLinkTransmitterShould : public ::testing::Test
{
public:
    DataLinkTransmitterShould() : logger_factory_(time_)
    {
    }

protected:
    stubs::TimeStub time_;
    using LoggerFactoryType = eul::logger::LoggerFactory<stubs::TimeStub, eul::logger::CurrentLoggingPolicy,
                                                         stubs::StandardErrorStreamStub>;
    LoggerFactoryType logger_factory_;
};

TEST_F(DataLinkTransmitterShould, StartTransmission)
{
    auto sut = object(interface<DataLinkTransmitter>{},
                      eul::mpl::types::bind_type<DataLinkTransmitterMembers::Logger>::to(
                          logger_factory_.create("DataLinkTransmitter")));

    // EXPECT_EQ(19, sut.print().a());
}

} // namespace msmp