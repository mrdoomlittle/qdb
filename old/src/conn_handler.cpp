# include "conn_handler.hpp"
boost::int8_t mdl::qgdb::conn_handler::add(boost::asio::ip::tcp::socket *__sock) {
	boost::thread* th = new boost::thread(boost::bind(&conn_handler::begin, this, __sock));
}
# define SET_MEM 0
# define GET_MEM 1
# define ADD_MEM 2
# include "set_info_t.hpp"

void mdl::qgdb::conn_handler::handle_memset(boost::asio::ip::tcp::socket *__sock) {
	boost::system::error_code any_error;

	serializer serialize('\0');
	qgdb::set_info_t set_info = {0, 0};

	std::size_t size = serialize.get_size(&set_info);
	printf("%d\n", size);
	serialize.init(size);
	serialize | 'w';

	boost::asio::read(*__sock, boost::asio::buffer(serialize.get_serial(), size), any_error);
	set_info.achieve(serialize);
	serialize.reset();

	char *mem_name = static_cast<char *>(malloc(set_info.mem_namelen * sizeof(char)));
	char *mem_value = static_cast<char *>(malloc(set_info.mem_valuelen * sizeof(char)));

	boost::asio::read(*__sock, boost::asio::buffer(mem_name, set_info.mem_namelen * sizeof(char)), any_error);
	boost::asio::read(*__sock, boost::asio::buffer(mem_value, set_info.mem_valuelen * sizeof(char)), any_error);

	this-> __this-> set_mem(mem_name, mem_value);

	std::free(mem_name);
	std::free(mem_value);

	if (serialize.get_serial() != nullptr)
		std::free(serialize.get_serial());
}

void mdl::qgdb::conn_handler::begin(boost::asio::ip::tcp::socket *__sock) {
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
				this-> handle_memset(__sock);
			break;
		}
		return;
	} while (true);
}
