# ifndef __connection__info__hpp
# define __connection__info__hpp
# include <boost/cstdint.hpp>
namespace mdl 
{
typedef struct {
    boost::uint16_t _port_number;
    char const * _ipv4_address;
    char const * _ipv6_address;
} connection_info;
}
typedef mdl::connection_info connection_info;
# endif /*__connection__info__hpp*/
