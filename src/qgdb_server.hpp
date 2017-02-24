# ifndef __qgdb__server__hpp
# define __qgdb__server__hpp
# include <boost/cstdint.hpp>
# include <boost/asio.hpp>
# include <boost/thread.hpp>

namespace mdl { class qgdb_server
{
	public:
	typedef struct {
		boost::uint16_t portno;
		char const *ipv4_addr;
		char const *ipv6_addr;
	} conn_info_t;

	qgdb_server(boost::asio::io_service& __io_service)
	: io_service(__io_service) {}

	boost::int8_t init(conn_info_t __conn_info);
	boost::int8_t begin();

	private:
	boost::asio::ip::tcp::endpoint *endpoint;
	boost::asio::io_service& io_service;


} ;
}

# endif /*__qgdb__server__hpp*/
