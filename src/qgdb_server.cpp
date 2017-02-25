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

		printf("waiting for connections.\n");
		acceptor.accept(*sock);
		printf("client has connected to database. addr: %s\n", sock-> remote_endpoint().address().to_string().c_str());

		conn_handler.add(sock, this);

	} while(1);
}



int main(int argc, char const *argv[]) {
	boost::asio::io_service io_service;
	mdl::qgdb_server qgdb_server(io_service);

	mdl::conn_info_t conn_info = {
		.portno = 21299,
		.ipv4_addr = "192.168.0.10"
	};

	qgdb_server.init(conn_info);
	qgdb_server.begin();
}
