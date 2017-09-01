# ifndef __conn__info__t__hpp
# define __conn__info__t__hpp
# include <boost/cstdint.hpp>
namespace mdl {
typedef struct {
	boost::uint16_t portno;
	char const *ipv4_addr;
	char const *ipv6_addr;
} conn_info_t;
}

# endif /*__conn__info__t__hpp*/
