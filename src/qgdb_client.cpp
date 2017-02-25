# include "qgdb_client.hpp"

boost::int8_t mdl::qgdb_client::init(conn_info_t __conn_info) {
	char *portno = to_string(__conn_info.portno);

	boost::asio::ip::tcp::resolver resolver(this-> io_service);
	boost::asio::ip::tcp::resolver::query query(__conn_info.ipv4_addr, portno);
	boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	static boost::asio::ip::tcp::socket sock(this-> io_service);

	sock.connect(*endpoint_iterator);
	this-> sock = &sock;

	std::free(portno);

	return 0;
}
void mdl::qgdb_client::set_mem(char const *__name, char const *__value) {
	qgdb::set_info_t set_info = {
		.mem_namelen = strlen(__name),
		.mem_valuelen = strlen(__value)
	};

	boost::system::error_code any_error;

	serializer serialize('\0');
	std::size_t size = serialize.get_size(&set_info);
	serialize.init(size);

	serialize | 'r';

	set_info.achieve(serialize);
	serialize.reset();

	boost::asio::write(*this-> sock, boost::asio::buffer(serialize.get_serial(), size), any_error);

	boost::asio::write(*this-> sock, boost::asio::buffer(__name, set_info.mem_namelen), any_error);
	boost::asio::write(*this-> sock, boost::asio::buffer(__value, set_info.mem_valuelen), any_error);

	std::free(serialize.get_serial());
}

boost::int8_t mdl::qgdb_client::begin() {
	boost::system::error_code any_error;

	serializer serialize('\0');
	qgdb::client_config_t client_config;

	std::size_t size = serialize.get_size(&client_config);
	serialize.init(size);

	serialize | 'w';

	boost::asio::read(*this-> sock, boost::asio::buffer(serialize.get_serial(), size), any_error);

	client_config.achieve(serialize);
	serialize.reset();

	boost::uint8_t instruct_id = 0;
	do {
		boost::asio::write(*this-> sock, boost::asio::buffer(&instruct_id, sizeof(boost::uint8_t)), any_error);

		switch(instruct_id) {
			case 0:
				this-> set_mem("Hello", "World");
			break;
		}

		return 0;

	} while(true);
}


int main(int argc, char const *argv[]) {
	boost::asio::io_service io_service;

	mdl::conn_info_t conn_info = {
		.portno = 21299,
		.ipv4_addr = "192.168.0.10"
	};

	mdl::qgdb_client qgdb_client(io_service);

	qgdb_client.init(conn_info);
	qgdb_client.begin();
}
