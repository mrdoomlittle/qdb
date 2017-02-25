# include "conn_handler.hpp"
boost::int8_t mdl::qgdb::conn_handler::add(boost::asio::ip::tcp::socket *__sock, qgdb_server *__this) {
	boost::thread* th = new boost::thread(boost::bind(&conn_handler::begin, this, __sock, __this));
}
# define SET_MEM 0
# define GET_MEM 1
void mdl::qgdb::conn_handler::begin(boost::asio::ip::tcp::socket *__sock, qgdb_server *__this) {
	boost::system::error_code any_error;

	serializer serialize('\0');
	qgdb::client_config_t client_config;

	std::size_t size = serialize.get_size(&client_config);
	serialize.init(size);

	serialize | 'r';

	client_config.achieve(serialize);
	serialize.reset();
	boost::asio::write(*__sock, boost::asio::buffer(serialize.get_serial(), size), any_error);
	if (any_error == boost::asio::error::eof) {
		fprintf(stderr, "failed to write to asio buffer.\n");
		return;
	}

	boost::uint8_t instruct_id = 0;
	do {
		boost::asio::read(*__sock, boost::asio::buffer(&instruct_id, sizeof(boost::uint8_t)), any_error);
		if (any_error == boost::asio::error::eof) {
			fprintf(stderr, "failed to read asio buffer.\n");
			return;
		}

		switch(instruct_id) {
			case SET_MEM:
				printf("set mem\n");
			break;

			case GET_MEM:
				printf("get mem\n");
			break;
		}

	} while (true);
}
