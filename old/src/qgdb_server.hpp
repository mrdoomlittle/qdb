# ifndef __qgdb__server__hpp
# define __qgdb__server__hpp
# include <boost/cstdint.hpp>
# include <boost/asio.hpp>
# include <boost/thread.hpp>
# include "conn_info_t.hpp"
namespace mdl { class qgdb_server
{
	public:
	qgdb_server(boost::asio::io_service& __io_service)
	: io_service(__io_service) {}

	boost::int8_t init(conn_info_t __conn_info);
	boost::int8_t begin();

	void set_mem(char const *__name, char const *__value);
	void get_mem();
	void add_mem();

	private:
	boost::asio::ip::tcp::endpoint *endpoint;
	boost::asio::io_service& io_service;


} ;
}

# endif /*__qgdb__server__hpp*/
