#define BOOST_NO_EXCEPTIONS

#include <cassert>

#include <boost/throw_exception.hpp>

void boost::throw_exception(std::exception const & e)
{
    (void)(e);
    assert(true);
}