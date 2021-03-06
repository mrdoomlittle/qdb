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

char *  mdl::qgdb_connect::db_var(char const * __var_type, char const * __var_name, 
char const * __var_value, tagged_memory & __tm, char * ex_space) {
    char * vt_tag = __tm.create_mem_tag("tm", __var_type);
    char * vn_tag = __tm.create_mem_tag("var_name", __var_name);
    char * vv_tag = nullptr;
    char * vs_tag = nullptr;
    if (__tm.compare_strings(__var_type, "get") == false) 
        vv_tag = __tm.create_mem_tag("var_value", __var_value);

    if (__tm.compare_strings(__var_type, "add"))
        vs_tag = __tm.create_mem_tag("var_space", ex_space);

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

    if (__tm.compare_strings(__var_type, "add")) {
        for (std::size_t o = 0 ; o != strlen(vs_tag) ; o ++) {
            buffer[i] = vs_tag[o];
            i++;
        }
    }
    bool err = false;
    this-> transmit_packet((* this-> socket), buffer, false, err);

    if (__tm.compare_strings(__var_type, "get") == true) {
        char * incomming = static_cast<char *>(malloc(BUFFER_LENGTH * sizeof(char)));
        memset(incomming, '\0', BUFFER_LENGTH * sizeof(char));

        boost::system::error_code error__;
        bool e = false;
        tmem_t * t = this-> receive_packet((* this-> socket), e);


        incomming = t-> dump_stack_memory(true);
        std::free(t);
      
        return incomming;       
    }

    std::free(vt_tag);
    std::free(vn_tag);
    if (__tm.compare_strings(__var_type, "get") == false)
        std::free(vv_tag);
    if (__tm.compare_strings(__var_type, "add"))
        std::free(vs_tag);

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
    return this-> receive_packet((* this-> socket), error, true);
}

mdl::tmem_t * mdl::qgdb_connect::recv_session_info(bool & error) {
    return this-> receive_packet((* this-> socket), error, true);
}

void mdl::qgdb_connect::login(char * __username, char * __password, tmem_t & __tm) {
    bool error = false;
    char * outgoing = this-> build_login_block(__username, __password, __tm);

    this-> transmit_packet((* this-> socket), outgoing, false, error);

    std::free(outgoing);
}

boost::uint8_t mdl::qgdb_connect::start(bool debug)
{
    this-> debug = debug;
    this-> set_connect_sstate(sevice_state::__is_running);
    
    /* you could say its a command line interpreter, but its poorly made
    * and it need updating and cleaning.
    */ 
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

    /* get the config file for the client from the server */
    tmem_t * config = this-> receive_config(e);

    config-> analyze_stack_memory(e);
    
    printf("dumping config stack.\n");

    config-> dump_stack_memory();

    /* store the motd in a char so we can write it to the terminal
    */   
    char * motd = config-> get_mem_value("motd", null_idc, e);

    /* write the motd to the terminal */
    cline.write_to_term(motd);

    /* free the memory that the motd used */
    std::free(motd);

    /* hear we will be able to store session info / login 
    * e.g. are we logged in or not ? and many other stuff
    */
    tmem_t * session_info = nullptr;

    /* get the read buffer length for the term from the config
    * that the server sent.
    */
    char * rbuff_len = config-> get_mem_value("term_rbuff_len", null_idc, e);

    /* change it to a int.
    */
    std::size_t rbl = atoi(rbuff_len);

    /* free the char memory that was used.
    */
    std::free(rbuff_len);

    tagged_memory::extra_options_t options;

    do
    {
        // NOTE: remove this as we dont need this, also
        // make tagged_memory 'create_mem_tag' accessabal without
        // needing to create a object of the class
        tagged_memory o(BUFFER_LENGTH, {}, options, this-> debug);
        bool error = false;

        session_info = this-> recv_session_info(error); 
        printf("dumping session stack.\n");
        session_info-> dump_stack_memory();

        char * access_state = session_info-> get_mem_value("allowed_access", null_idc, error, 0, true);
        
        if (session_info-> compare_strings(access_state, "no")) 
            is_logged_in = false;
        else if (session_info-> compare_strings(access_state, "yes"))
            is_logged_in = true;

        std::free(access_state);
        relog:
        char * term_input = cline.read_from_term(rbl, true);
        cline.filter(true);
   
        std::free(term_input);

        printf("testing\n");

        
        if (cline.is_base_instruct(base_instruct))
        {
            /* NOTE: all other commands must go after the login one as 
            * the client need access before its allows to access the other
            * commands, if not the read / write might become out of sync.
            */
            if (cline.is_bi_argument("login") && is_logged_in == false) {
                char * username = cline.get_bi_arg_value("--uname");
                char * password = cline.get_bi_arg_value("--passwd");

                char * outgoing = this-> build_login_block(username, password, o);


                this-> transmit_packet((* this-> socket), outgoing, false, error);
              
                std::free(username);
                std::free(password);
                std::free(outgoing);
            }

            if (cline.is_bi_argument("login") == false && is_logged_in == false) { 
                printf("sorry but other commands will not work unless you are logged in to the database.\n"); 
                goto relog; 
            }

            /* as set and add take the same arguments theres no point
            * of having then in seprit if sectors.
            */
            if (cline.is_bi_argument("add") || cline.is_bi_argument("set")) {
                /* get the argument value thats named 'name'
                */
                char * mem_name = cline.get_bi_arg_value("--name");

                /* get the argument value thats named 'value'
                */
                char * mem_value = cline.get_bi_arg_value("--value");

                char * mem_space = nullptr;

                if (cline.is_bi_argument("add")) 
                    mem_space = cline.get_bi_arg_value("--space");

                /* as we dont know what the terminal message is
                * we have to declear it manulay.
                */
                if (cline.is_bi_argument("add"))
                    this-> db_var("add", mem_name, mem_value, o, mem_space);

                if (cline.is_bi_argument("set"))
                    this-> db_var("set", mem_name, mem_value, o);

                /* free the used memory
                */
                std::free(mem_name);
                std::free(mem_value);
           
            } else if (cline.is_bi_argument("get")) {
                char * mem_name = cline.get_bi_arg_value("--name");
            
                char * incomming = this-> db_var("get", mem_name, "", o);

                cline.write_to_term("resault from serve is &\n", incomming);

                std::free(mem_name);
            }
        }

        std::free(session_info);
    } while (this-> is_connect_sstate(sevice_state::__is_running));

    return 0;
}
/*
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
    qgdb_connect.start(atoi(arg_v[3]));
}
*/
