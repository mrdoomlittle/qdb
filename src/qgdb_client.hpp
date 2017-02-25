# ifndef __qgdb__client__hpp
# define __qgdb__client__hpp
# include <boost/cstdint.hpp>
# include <boost/asio.hpp>
# include "conn_info_t.hpp"
# include <to_string.hpp>
# include "session_info_t.hpp"
# include "client_config_t.hpp"
# include <serializer.hpp>
# include "set_info_t.hpp"
# include <string.h>
namespace mdl { class qgdb_client
{
	public:
	qgdb_client(boost::asio::io_service& __io_service)
	: io_service(__io_service) {}

	boost::int8_t init(conn_info_t __conn_info);
	boost::int8_t begin();

	void set_mem(char const *__name, char const *__value);
	void get_mem(char const *__value);
	void add_mem(char const *__name);
	private:
	boost::asio::ip::tcp::socket* sock;
	boost::asio::io_service& io_service;
} ;
}

# endif /*__qgdb__client__hpp*/
