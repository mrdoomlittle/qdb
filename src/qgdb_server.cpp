# include "qgdb_server.hpp"
# include "conn_handler.hpp"
# include <strcmb.hpp>
boost::int8_t mdl::qgdb_server::init(conn_info_t __conn_info) {
	static boost::asio::ip::tcp::endpoint endpoint(
		boost::asio::ip::address_v4::from_string(__conn_info.ipv4_addr), __conn_info.portno
	);

	this-> endpoint = &endpoint;
}

boost::int8_t mdl::qgdb_server::begin() {
	boost::asio::ip::tcp::acceptor acceptor(this-> io_service, (*this-> endpoint));
	qgdb::conn_handler conn_handler;
	conn_handler.__this = this;

	do {
		boost::asio::ip::tcp::socket *sock = new boost::asio::ip::tcp::socket(this-> io_service);

		printf("waiting for connections.\n");
		acceptor.accept(*sock);
		printf("client has connected to database. addr: %s\n", sock-> remote_endpoint().address().to_string().c_str());

		conn_handler.add(sock);

	} while(1);
}

# include <unistd.h>
void mdl::qgdb_server::set_mem(char const *__name, char const *__value) {
	char *file_name = strcmb("data/", const_cast<char *>(__name), STRCMB_FREE_NONE);

	if (access(file_name, F_OK) == -1) {
		fprintf(stderr, "file does not exist, so we cant set anything.\n");
		std::free(file_name);
		return;
	}

	FILE* file = fopen(file_name, "wb");

	std::size_t vallen = strlen(__value);

	fwrite(__value, sizeof(char), vallen, file);

	fclose(file);

	std::free(file_name);

	printf("mem set, name = %s | value = %s\n", __name, __value);
}

# include <sys/types.h>
# include <dirent.h>
# include <errno.h>

int main(int argc, char const *argv[]) {
	// check if data directory exists
	DIR* dir = opendir("data");

	if (!dir) {
		if (errno != ENOENT) {
			closedir(dir);
			return 1;
		}

		fprintf(stderr, "error: data directory does not exist.\n");
		closedir(dir);
		return 1;
	}

	closedir(dir);

	boost::asio::io_service io_service;
	mdl::qgdb_server qgdb_server(io_service);

	mdl::conn_info_t conn_info = {
		.portno = 21299,
		.ipv4_addr = "192.168.0.10"
	};

	qgdb_server.init(conn_info);
	qgdb_server.begin();

	return 0;
}
