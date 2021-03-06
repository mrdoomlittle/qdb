# include "qgdb_deamon.hpp"
# include <iostream>
# include <cstdio>
# include "conn_handler.hpp"
/*
NOTE: im not using async
*/

boost::uint8_t mdl::qgdb_deamon::initialize(connection_info cinfo)
{
    static boost::asio::ip::tcp::endpoint __endpoint(
        boost::asio::ip::address_v4::from_string(cinfo._ipv4_address), cinfo._port_number);

    this-> endpoint = &__endpoint;

    tagged_memory::extra_options_t extra_options;

    this-> db_memory = new tmem_t(DB_MEM_LENGTH, {}, extra_options, true);

    /* load database memory into stack so we can use it later
    */
    this-> db_memory-> load_mem_stack_from_file("", "db_memory.qg_db");

    /*
    * database layout:
    * .client config
    * .server config
    * .db config
    */

    this-> db_config = new tmem_t(DB_MEM_LENGTH, {}, extra_options, false);
    this-> client_config = new tmem_t(DB_MEM_LENGTH, {}, extra_options, false);
    this-> server_config = new tmem_t(DB_MEM_LENGTH, {}, extra_options, false);

    bool error = false;

    this-> db_memory-> analyze_stack_memory(error);

    this-> load_db_config(error);
    this-> load_client_config(error);
    this-> load_server_config(error);

    return 0;
}

void mdl::qgdb_deamon::load_db_config(bool & __error) {
    char * db_config = this-> db_memory-> get_mem_value("db_config", null_idc, __error);

    this-> db_config-> dump_into_stack(db_config);
    this-> db_config-> analyze_stack_memory(__error);

    std::free(db_config);
}

void mdl::qgdb_deamon::load_client_config(bool & __error) {
    char * client_config = this-> db_memory-> get_mem_value("client_config", null_idc, __error);

    this-> client_config-> dump_into_stack(client_config);
    this-> client_config-> analyze_stack_memory(__error);

    std::free(client_config);
}

void mdl::qgdb_deamon::load_server_config(bool & __error) {
    char * server_config = this-> db_memory-> get_mem_value("server_config", null_idc, __error);

    this-> server_config-> dump_into_stack(server_config);
    this-> server_config-> analyze_stack_memory(__error);

    std::free(server_config);
}

void mdl::qgdb_deamon::send_client_config(
    boost::asio::ip::tcp::socket & __socket, bool& __error) {
    this-> transmit_packet(__socket, this-> client_config, false, __error);
}

void mdl::qgdb_deamon::send_session_info(tmem_t * __session_info,
    boost::asio::ip::tcp::socket & __socket, bool& __error) {

    this-> transmit_packet(__socket, __session_info, false, __error);
}

boost::uint8_t mdl::qgdb_deamon::start(boost::thread ** __t)
{
    boost::asio::ip::tcp::acceptor acceptor(this-> io_service, (*this-> endpoint));

    this-> set_deamon_sstate(sevice_state::__is_running);

    conn_handler connection;

    do
    {
        boost::asio::ip::tcp::socket * sock = new boost::asio::ip::tcp::socket(this-> io_service);

        boost::asio::ip::tcp::socket & socket = *sock;

        if (this-> accept_incomming(socket, acceptor) == 1) return 1;

        printf("adding a connection.\n");

        connection.add_connection(sock, this);

    } while (this-> is_demaon_sstate(sevice_state::__is_running));

    if (this-> is_demaon_sstate(sevice_state::__not_running)) {
        pthread_cancel((* __t)-> native_handle());
    }

    return 0;
}

boost::uint8_t mdl::qgdb_deamon::accept_incomming(
    boost::asio::ip::tcp::socket & __socket,
    boost::asio::ip::tcp::acceptor & __acceptor)
{
    /* accept incomming connection
    */
    __acceptor.accept(__socket);

    return 0;
}

# include <clinterp.hpp>
void mdl::qgdb_deamon::terminal(boost::thread ** __t) {
    clinterp cl;

    cl.add_base_instruct("qgdb");
    cl.add_bi_argument("qgdb", "stop");

    printf("Welcome to QGDB. if you need help type 'help'\n");
    do
    {
        char * tmp = cl.read_from_term(1048, true);
        cl.filter(true);
        std::free(tmp);

        if (cl.is_base_instruct("qgdb")) {
            if (cl.is_bi_argument("help")) {
                printf("stop [no args]\nadd [name=, value=]\nset [name=, value=]\nget [name=]\n");
            }
            else if (cl.is_bi_argument("add")) {
                char * var_name = cl.get_bi_arg_value("name");
                char * var_value = cl.get_bi_arg_value("value");

                bool error = false;
                this-> db_memory-> add_mem_tag(var_name, var_value, 0, error);

                std::free(var_name);
                std::free(var_value);
            }
            else if (cl.is_bi_argument("set")) {
                char * var_name = cl.get_bi_arg_value("name");
                char * var_value = cl.get_bi_arg_value("value");

                bool error = false;
                this-> db_memory-> set_mem_value(var_name, var_value, null_idc, error);

                std::free(var_name);
                std::free(var_value);
            }
            else if (cl.is_bi_argument("get")) {
                char * var_name = cl.get_bi_arg_value("name");
                bool error = false;
                char * var_value = this-> db_memory-> get_mem_value(var_name, null_idc, error, 0, true);

                printf("resault from database is %s\n", var_value);

                std::free(var_name);
                std::free(var_value);
            }
        }
    } while(this-> is_demaon_sstate(sevice_state::__is_running));

    if (this-> is_demaon_sstate(sevice_state::__not_running)) {

        pthread_cancel((* __t)-> native_handle());
    }

}

int main (int arg_c, char * arg_v [])
{
    if (arg_c < 2) {
        printf("there must be 2 arguments for this to work.\n");
        return 1;
    }

    boost::asio::io_service io_service;

    mdl::connection_info cinfo;
    cinfo._port_number = 21299;
    cinfo._ipv4_address = arg_v[1];
    mdl::qgdb_deamon qgdb_deamon(io_service);

    qgdb_deamon.initialize(cinfo);

    boost::thread * t1;
    t1 = new boost::thread(boost::bind(&mdl::qgdb_deamon::start, &qgdb_deamon, &t1));
    boost::thread * t2;
    t2 = new boost::thread(boost::bind(&mdl::qgdb_deamon::terminal, &qgdb_deamon, &t2));
    t1-> join();
    t2-> join();
}
