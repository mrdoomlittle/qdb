# ifndef __conn__handler__hpp
# define __conn__handler__hpp
# include <boost/thread.hpp>
# include <boost/cstdint.hpp>
# include <boost/asio.hpp>
# include "qgdb_server.hpp"
# include <serializer.hpp>
# include "client_config_t.hpp"
# include <cstdio>
namespace mdl { namespace qgdb { class conn_handler
{
	public:
	boost::int8_t add(boost::asio::ip::tcp::socket *__sock, qgdb_server *__this);
	void begin(boost::asio::ip::tcp::socket *__sock, qgdb_server *__this);
} ;
}
}



# endif /*__conn__handler__hpp*/
