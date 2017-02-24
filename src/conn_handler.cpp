# include "conn_handler.hpp"
boost::int8_t mdl::qgdb::conn_handler::add(boost::asio::ip::tcp::socket *__sock, qgdb_server *__this) {
	boost::thread* th = new boost::thread(boost::bind(&conn_handler::begin, this, __sock, __this));
}

void mdl::qgdb::conn_handler::begin(boost::asio::ip::tcp::socket *__sock, qgdb_server *__this) {
	do {


	} while (true);
}
