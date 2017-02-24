# include "qgdb_server.hpp"
# include "conn_handler.hpp"
boost::int8_t mdl::qgdb_server::init(conn_info_t __conn_info) {
	static boost::asio::ip::tcp::endpoint endpoint(
		boost::asio::ip::address_v4::from_string(__conn_info.ipv4_addr), __conn_info.portno
	);

	this-> endpoint = &endpoint;
}

boost::int8_t mdl::qgdb_server::begin() {
	boost::asio::ip::tcp::acceptor acceptor(this-> io_service, (*this-> endpoint));
	qgdb::conn_handler conn_handler;

	do {
		boost::asio::ip::tcp::socket *sock = new boost::asio::ip::tcp::socket(this-> io_service);

		acceptor.accept(*sock);

		conn_handler.add(sock, this);

	} while(1);
}



int main(int argc, char const *argv[]) {


}
