# include "qgdb_deamon.hpp"
# include <iostream>
# include <cstdio>
/*
NOTE: im not using async
*/

boost::uint8_t mdl::qgdb_deamon::initialize(connection_info cinfo)
{
    static boost::asio::ip::tcp::endpoint __endpoint(
        boost::asio::ip::address_v4::from_string(cinfo._ipv4_address), cinfo._port_number);

    this-> endpoint = &__endpoint;

    this-> db_memory = new tmem_t(DB_MEM_LENGTH, {':', ';', '~'}, true);

    /* load database memory into stack so we can use it later
    */
    this-> db_memory-> load_mem_stack_from_file("db_memory.db");

    /*
    * database layout:
    * .client config
    * .server config
    * .db config
    */

    this-> db_config = new tmem_t(DB_MEM_LENGTH, {':', ';', '~'}, false);
    this-> client_config = new tmem_t(DB_MEM_LENGTH, {':', ';', '~'}, false);
    this-> server_config = new tmem_t(DB_MEM_LENGTH, {':', ';', '~'}, false);
    
    bool error = false;
    this-> db_memory-> analyze_stack_memory(error);

    this-> load_db_config(error);
    
    this-> load_client_config(error);
    this-> load_server_config(error);

    return 0;
}

void mdl::qgdb_deamon::load_db_config(bool & __error) {
    char * db_config = this-> db_memory-> get_mem_value("db_config", __error);

    this-> db_config-> dump_into_stack(db_config);
    this-> db_config-> analyze_stack_memory(__error);

    std::free(db_config);
}

void mdl::qgdb_deamon::load_client_config(bool & __error) {
    char * client_config = this-> db_memory-> get_mem_value("client_config", __error);

    this-> client_config-> dump_into_stack(client_config);
    this-> client_config-> analyze_stack_memory(__error);

    std::free(client_config);
}

void mdl::qgdb_deamon::load_server_config(bool & __error) {
    char * server_config = this-> db_memory-> get_mem_value("server_config", __error);

    this-> server_config-> dump_into_stack(server_config);
    this-> server_config-> analyze_stack_memory(__error);

    std::free(server_config);
}

void mdl::qgdb_deamon::send_client_config(
    boost::asio::ip::tcp::socket & __socket) {
    this-> transmit_packet(__socket, this-> client_config);
}

void mdl::qgdb_deamon::send_session_info(tmem_t * __session_info,
    boost::asio::ip::tcp::socket & __socket) {

    this-> transmit_packet(__socket, __session_info);
}

boost::uint8_t mdl::qgdb_deamon::start(boost::thread ** __t)
{
    boost::asio::ip::tcp::acceptor acceptor(this-> io_service, (*this-> endpoint));
    
    this-> set_deamon_sstate(sevice_state::__is_running);

    /* login manager. this will manage the user login stuff e.g. password usernames etc
    */
    login_manager lmanager;

    login_manager::session & s = lmanager.get_session();
   
    bool e = false; 
    tmem_t session_info(128, {':', ';', '~'}, true);
    
    /* NOTE: fore some resion add_mem_tag dose not work 
    * before analyzeing.
    */
    session_info.dump_into_stack(":allowed_access~no;");
    session_info.analyze_stack_memory(e);

    do
    {
        boost::asio::ip::tcp::socket socket(this-> io_service);

        if (this-> accept_incomming(socket, acceptor) == 1) return 1; 
        
        printf("a client has connected to the database\n");    
        this-> send_client_config(socket);
    
        for (;;) {
            bool error = false;

            this-> send_session_info(&session_info, socket);
         
            /* recive the incomming packet from client.
            */ 
            tagged_memory * packet = this-> receive_packet(socket, error);
        
            /* if there was a error well receiving the packet or somthing else,
            * stop any further action
            */ 
            if (error == true) break;
            std::cout << "________________________________" << "\n";
            char * si = session_info.dump_stack_memory();
            std::free(si);
            char * username;
            char * password;

            if (s.is_logged_in == false)
            {           
                username = packet-> get_mem_value("username", error, 0, true);      
                password = packet-> get_mem_value("password", error, 0, true);

                printf("checking username and password\n");
                if (lmanager.check_login_data(username, password, error))
                    lmanager.start_session(username, password); 
                
                std::cout << "uname: " << username << ", passwd: " << password << std::endl;

                if (s.is_logged_in)
                    session_info.set_mem_value("allowed_access", "yes", e);

                printf("password was %s\n", s.is_logged_in == false? "incorrect" : "correct");
            }
            
            if (s.is_logged_in == false || s.just_logged_in) {
                if (s.is_logged_in == false) {
                    std::free(username);
                    std::free(password);
                }

                std::free(packet);

                if (s.just_logged_in)
                    s.just_logged_in = false;
                                    
                continue;
            }
            
           
            printf("user %s logged into the database.\n", s.username);

            char * val = packet-> get_mem_value("tm", error, 0, true);

            printf("terminal command: %s\n", val);

            if (packet-> compare_strings(val, "add")) {
                printf("%s has called for db to add a var.\n", s.username);
                /* get the memory name and its value, as we need it. */
                char * mem_name = packet-> get_mem_value("var_name", error, 0, true);
                char * mem_value = packet-> get_mem_value("var_value", error, 0, true);

                /* add a memory tag to the stack. */
                this-> db_memory-> add_mem_tag(mem_name, mem_value, 0, error);
             
                printf("var name: %s, var value: %s\n", mem_name, mem_value);
                /* free the memory used to store the name and val of var */
                std::free(mem_name);
                std::free(mem_value);
            } else if (packet-> compare_strings(val, "set")) {     
                printf("%s has called for db to set a var.\n", s.username);
                char * mem_name = packet-> get_mem_value("var_name", error, 0, true);
                char * mem_value = packet-> get_mem_value("var_value", error, 0, true);

                this-> db_memory-> set_mem_value(mem_name, mem_value, error);        

                printf("var name: %s, var value: %s\n", mem_name, mem_value);
                /* free the memory used to store the name and val of var */
                std::free(mem_name);
                std::free(mem_value);
            } else if (packet-> compare_strings(val, "get")) {
                printf("%s has called for db to get a var.\n", s.username);
                char * mem_name = packet-> get_mem_value("var_name", error, 0, true);
                char * mem_value = this-> db_memory-> get_mem_value(mem_name, error, 0, true);
  
                this-> transmit_packet(socket, mem_value);

                printf("var name: %s, var value: %s\n", mem_name, mem_value);
                /* free the memory used to store the name and val of var */
                std::free(mem_name);
                std::free(mem_value);
            } 

            std::free(val);
            std::free(packet);       
        }

        lmanager.end_session();
        this-> db_memory-> save_mem_stack_to_file("db_memory.db"); 

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
            if (cl.is_bi_argument("stop")) {
                // NOTE: does not work
                printf("stopping server.\n");
                this-> set_deamon_sstate(sevice_state::__not_running);
                printf("res = %d\n", this-> get_deamon_sstate());
            }
            else if (cl.is_bi_argument("help")) {
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
                this-> db_memory-> set_mem_value(var_name, var_value, error); 

                std::free(var_name);
                std::free(var_value);
            }
            else if (cl.is_bi_argument("get")) {
                char * var_name = cl.get_bi_arg_value("name");
                bool error = false;
                char * var_value = this-> db_memory-> get_mem_value(var_name, error, 0, true);

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
