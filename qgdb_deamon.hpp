# ifndef __qgdb__deamon__hpp
# define __qgdb__deamon__hpp
# include <boost/cstdint.hpp>
# include <boost/cstdlib.hpp>
# include <boost/asio.hpp>
# include "comm_handler.hpp"
# include <boost/thread.hpp>
# include <iostream>
# include "login_manager.hpp"
# include <tagged_memory.hpp>
# include "connection_info.hpp"
# define DB_MEM_LENGTH 30720
namespace mdl { class qgdb_deamon : public comm_handler
{
    public:

    qgdb_deamon(boost::asio::io_service & __io_service)
    : io_service(__io_service) { }
  
    boost::uint8_t accept_incomming(
        boost::asio::ip::tcp::socket & __socket,
        boost::asio::ip::tcp::acceptor & __acceptor);

    boost::uint8_t initialize(connection_info cinfo);
    boost::uint8_t start(boost::thread ** __t);
  
    bool is_demaon_sstate(boost::uint8_t __deamon_sstate) {
        return this-> deamon_sstate == __deamon_sstate? true : false;
    }

    void set_deamon_sstate(boost::uint8_t __deamon_sstate) {
        this-> deamon_sstate = __deamon_sstate;
    }

    boost::uint8_t get_deamon_sstate() {
        return this-> deamon_sstate;
    }

    void terminal(boost::thread ** __t);
    void send_client_config(
        boost::asio::ip::tcp::socket & __socket);
    void send_session_info(tmem_t * __session_info,
    boost::asio::ip::tcp::socket & __socket);

    void load_db_config(bool & __error);
    void load_client_config(bool & __error);
    void load_server_config(bool & __error);
     
    private: 
    tmem_t * db_memory;
    tmem_t * db_config;

    tmem_t * client_config;
    tmem_t * server_config;
 
    enum sevice_state : boost::uint8_t { __is_running, __not_running };
    boost::uint8_t deamon_sstate = sevice_state::__not_running;

    boost::asio::io_service & io_service;
    boost::asio::ip::tcp::endpoint * endpoint;
} ;
}

# endif /*__qgdb__deamon__hpp*/
