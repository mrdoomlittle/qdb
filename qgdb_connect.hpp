# ifndef __qgdb__connect__hpp
# define __qgdb__connect__hpp
# include "comm_handler.hpp"
# include "connection_info.hpp"
# include <boost/cstdint.hpp>
# include <boost/cstdlib.hpp>
# include <boost/asio.hpp>
//# include <tagmem.hpp>
namespace mdl { class qgdb_connect : public comm_handler
{
    public:
    qgdb_connect(boost::asio::io_service & __io_service)
    : io_service(__io_service) { }

    boost::uint8_t initialize(connection_info cinfo);
    boost::uint8_t start(bool debug);

    bool is_connect_sstate(boost::uint8_t __connect_sstate) {
        return this-> connect_sstate == __connect_sstate? true : false;
    }

    void set_connect_sstate(boost::uint8_t __connect_sstate) {
        this-> connect_sstate = __connect_sstate;
    }

    boost::uint8_t get_connect_sstate() {
        return this-> connect_sstate;
    }

    char * db_var(char const * __var_type, char const * __var_name, 
        char const * __var_value, tagged_memory & __tm, char * ex_space = nullptr);

    char * build_login_block(char * __uname, char * __passwd, tagged_memory & __tm);

    tmem_t * recv_session_info(bool & error);
    tmem_t * receive_config(bool & error);
  
    private:
    bool debug = false;

    boost::asio::ip::tcp::socket * socket;
    boost::asio::io_service & io_service;
    enum sevice_state : boost::uint8_t { __is_running, __not_running };
    boost::uint8_t connect_sstate = sevice_state::__not_running;    
} ; 

}

# endif /*__qgdb__connect__hpp*/
