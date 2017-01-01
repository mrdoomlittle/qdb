# include <iostream>
# include <sstream>
# include <string.h>
# include "qgdb_connect.hpp"
# include <cstdio>
# include <tagged_memory.hpp>
# include <clinterp.hpp>
# include <string.h>
//# define BUFFER_LENGTH 1048

boost::uint8_t mdl::qgdb_connect::initialize(connection_info cinfo)
{
    std::stringstream ss;
    ss << cinfo._port_number;

    const char * tmp = ss.str().c_str();

    boost::asio::ip::tcp::resolver resolver(this-> io_service);
    boost::asio::ip::tcp::resolver::query query(cinfo._ipv4_address, tmp);
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    static boost::asio::ip::tcp::socket _socket(this-> io_service);
    _socket.connect(*endpoint_iterator);
    this-> socket  = &_socket;

    return 0;
}

char *  mdl::qgdb_connect::db_var(char const * __var_type, char const * __var_name, char const * __var_value, tagged_memory & __tm) {
    char * vt_tag = __tm.create_mem_tag("tm", __var_type);
    char * vn_tag = __tm.create_mem_tag("var_name", __var_name);
    char * vv_tag = nullptr;
    if (__tm.compare_strings(__var_type, "get") == false) 
        vv_tag = __tm.create_mem_tag("var_value", __var_value);

    std::size_t length = 0;
    if (__tm.compare_strings(__var_type, "get") == false)
        length = (strlen(vt_tag) + strlen(vn_tag) + strlen(vv_tag));
    else
        length = (strlen(vt_tag) + strlen(vn_tag));

    char * buffer = static_cast<char *>(malloc(length * sizeof(char) + 1));
    memset(buffer, '\0', length * sizeof(char) + 1);

    std::size_t i = 0;
    for (std::size_t o = 0 ; o != strlen(vt_tag) ; o ++) {
        buffer[i] = vt_tag[o];
        i++;
    }

    for (std::size_t o = 0 ; o != strlen(vn_tag) ; o ++) {
        buffer[i] = vn_tag[o];
        i++;
    }

    if (__tm.compare_strings(__var_type, "get") == false) {
        for (std::size_t o = 0 ; o != strlen(vv_tag) ; o ++) {
            buffer[i] = vv_tag[o];
            i++;
        }
    }

    this-> transmit_packet((* this-> socket), buffer);
//    boost::asio::write((* this-> socket), boost::asio::buffer(buffer, strlen(buffer)));

    if (__tm.compare_strings(__var_type, "get") == true) {
        char * incomming = static_cast<char *>(malloc(BUFFER_LENGTH * sizeof(char)));
        memset(incomming, '\0', BUFFER_LENGTH * sizeof(char));
        std::size_t icomm_buff_len = 0;

        boost::system::error_code error__;
        bool e = false;
        tmem_t * t = this-> receive_packet((* this-> socket), e);


        incomming = t-> dump_stack_memory();
        std::free(t);
        //icomm_buff_len = this-> socket-> read_some(boost::asio::buffer(incomming, BUFFER_LENGTH), error__);

        return incomming;       
    }

    std::free(vt_tag);
    std::free(vn_tag);
    if (__tm.compare_strings(__var_type, "get") == false)
        std::free(vv_tag);

    return nullptr;
}

char * mdl::qgdb_connect::build_login_block(char * __uname, char * __passwd, tagged_memory & __tm){
    char * username = __tm.create_mem_tag("username", __uname);
    char * password = __tm.create_mem_tag("password", __passwd);

    std::size_t length = std::strlen(username) + std::strlen(password);

    char * buffer = static_cast<char *>(malloc(length * sizeof(char) + 1));
    memset(buffer, '\0', length * sizeof(char) + 1);

    std::size_t i = 0;
    for (std::size_t o = 0 ; o != std::strlen(username) ; o ++) {
        buffer[i] = username[o];
        i++;
    }

    for (std::size_t o = 0 ; o != std::strlen(password) ; o ++) {
        buffer[i] = password[o];
        i++;
    }

    std::free(username);
    std::free(password);

    return buffer;
}

mdl::tmem_t * mdl::qgdb_connect::receive_config(bool & error) {
    return this-> receive_packet((* this-> socket), error);
}

mdl::tmem_t * mdl::qgdb_connect::recv_session_info(bool & error) {
    return this-> receive_packet((* this-> socket), error);
}

boost::uint8_t mdl::qgdb_connect::start()
{
    this-> set_connect_sstate(sevice_state::__is_running);
   
    clinterp cline;
    
    /* the base instructer. this just makes it easer to change.
    */
    char const * base_instruct = "qgdb";

    /* base instructer aka the base command
    * it sits at the base of the arguments
    */
    cline.add_base_instruct(base_instruct);

    /* arguments of the base instructer
    */
    cline.add_bi_argument(base_instruct, "set");
    cline.add_bi_argument(base_instruct, "get"); 
    cline.add_bi_argument(base_instruct, "add");

    bool is_logged_in = false, e = false;

    tmem_t * config = this-> receive_config(e);
    config-> analyze_stack_memory(e);
    
    printf("dumping config stack.\n");

    config-> dump_stack_memory();
   
    char * motd = config-> get_mem_value("motd", e);

    cline.write_to_term(motd);
    std::free(motd);

    tmem_t * session_info = nullptr;

    do
    {
        tagged_memory o(BUFFER_LENGTH, {':', ';', '~'}, true);
        bool error = false;

        session_info = this-> recv_session_info(error); 
        printf("dumping session stack.\n");
        session_info-> dump_stack_memory();

        char * access_state = session_info-> get_mem_value("allowed_access", error, 0, true);
        
        if (session_info-> compare_strings(access_state, "no")) 
            is_logged_in = false;
        else if (session_info-> compare_strings(access_state, "yes"))
            is_logged_in = true;

        std::free(access_state);
        relog:
        char * term_input = cline.read_from_term(1048, true);
        cline.filter(true);
   
        std::free(term_input);

        printf("testing\n");

        
        if (cline.is_base_instruct(base_instruct))
        {
            if (cline.is_bi_argument("login") && is_logged_in == false) {
                char * username = cline.get_bi_arg_value("uname");
                char * password = cline.get_bi_arg_value("passwd");

                char * outgoing = this-> build_login_block(username, password, o);


                this-> transmit_packet((* this-> socket), outgoing);
              
                std::free(username);
                std::free(password);
                std::free(outgoing);
            }

            if (cline.is_bi_argument("login") == false && is_logged_in == false) { 
                printf("sorry by other commands will not work unless to login to the database.\n"); 
                goto relog; 
            }

            /* as set and add take the same arguments theres no point
            * of having then in seprit if sectors.
            */
            if (cline.is_bi_argument("add") || cline.is_bi_argument("set")) {
                /* get the argument value thats named 'name'
                */
                char * mem_name = cline.get_bi_arg_value("name");

                /* get the argument value thats named 'value'
                */
                char * mem_value = cline.get_bi_arg_value("value");

                /* as we dont know what the terminal message is
                * we have to declear it manulay.
                */
                if (cline.is_bi_argument("add"))
                    this-> db_var("add", mem_name, mem_value, o);

                if (cline.is_bi_argument("set"))
                    this-> db_var("set", mem_name, mem_value, o);

                /* free the used memory
                */
                std::free(mem_name);
                std::free(mem_value);
           
            } else if (cline.is_bi_argument("get")) {
                char * mem_name = cline.get_bi_arg_value("name");
            
                char * incomming = this-> db_var("get", mem_name, "", o);

                cline.write_to_term("resault from serve is &\n", incomming);

                std::free(mem_name);
            }
        }

        std::free(session_info);
    } while (this-> is_connect_sstate(sevice_state::__is_running));

    return 0;
}



int main(int arg_c, char * arg_v[])
{
    if (arg_c < 3) {
        printf("needs more then %d arguments to work\n", arg_c);
        return 1;
    }

    boost::asio::io_service io_service;
    mdl::connection_info cinfo;
    cinfo._port_number = atoi(arg_v[2]);
    cinfo._ipv4_address = arg_v[1];
    mdl::qgdb_connect qgdb_connect(io_service);
    qgdb_connect.initialize(cinfo);
    qgdb_connect.start();
}
