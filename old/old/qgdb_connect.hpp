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

    void login(char * __username, char * __password, tmem_t & __tm);

    void add(char const * __mem_name, char const * __mem_value, std::size_t __ex_space, tagged_memory & __tm) {
        std::string st = std::to_string(__ex_space);

        char * c = const_cast<char *>(st.c_str());

        this-> db_var("add", __mem_name, __mem_value, __tm, c);
    }

    char * get(char const * __mem_name, tagged_memory & __tm) {
        return this-> db_var("get", __mem_name, nullptr, __tm, nullptr);
    }
    
    void set(char const * __mem_name, char const * __mem_value, tagged_memory & __tm) {
        this-> db_var("set", __mem_name, __mem_value, __tm, nullptr);
    }

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
