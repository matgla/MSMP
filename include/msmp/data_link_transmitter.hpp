#pragma once

#include <eul/logger/logger_traits.hpp>
#include <eul/mpl/mixin/access.hpp>
#include <eul/mpl/mixin/name.hpp>
#include <eul/mpl/mixin/object.hpp>


namespace msmp
{

struct DataLinkTransmitterMembers
{
    class Logger;
};

template <typename BaseType>
class DataLinkTransmitter : DataLinkTransmitterMembers
{
public:
    DataLinkTransmitter() : data_(this)
    //, logger_(data_[eul::mpl::mixin::name<Logger>{}])
    {
        // logger_.info() << "Hello";
    }


private:
    eul::mpl::mixin::access<BaseType> data_;
    eul::mpl::mixin::access<BaseType>::get_binded<Logger>()::type& logger_;
};

} // namespace msmp