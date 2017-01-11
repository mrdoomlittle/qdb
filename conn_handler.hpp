# ifndef __conn__handler__hpp
# define __conn__handler__hpp
# include <vector>
namespace mdl { class conn_handler {
    public :
    
    void manage_connection(boost::thread ** __th, boost::asio::ip::tcp::socket * __socket, qgdb_deamon * _this) {
        printf("a client has connected to the database\n");
        boost::asio::ip::tcp::socket & socket = *__socket;
        /* login manager. this will manage the user login stuff e.g. password usernames etc
        */
        login_manager lmanager;

        login_manager::session & s = lmanager.get_session();
   
        bool e = false, error = false; 
        tmem_t session_info(128, {':', '~', ';'}, true);
    
        /* NOTE: fore some resion add_mem_tag dose not work 
        * before analyzeing.
        */
        session_info.dump_into_stack(":allowed_access~no;");
        session_info.analyze_stack_memory(e);

        _this-> send_client_config(socket, error);
        if (error == true) goto end;
        for (;;) {
            error = false;

            _this-> send_session_info(&session_info, socket, error);

            if (error == true) break;        
            /* recive the incomming packet from client.
            */ 
            tmem_t * packet = _this-> receive_packet(socket, error);
    
            /* if there was a error well receiving the packet or somthing else,
            * stop any further action
            */ 
            if (error == true) break;
            std::cout << "________________________________" << "\n";
            char * si = session_info.dump_stack_memory(true);
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
                char * mem_space = packet-> get_mem_value("var_space", error, 0, true);
                /* add a memory tag to the stack. */
                _this-> db_memory-> add_mem_tag(mem_name, mem_value, atoi(mem_space), error);
             
                printf("var name: %s, var value: %s\n", mem_name, mem_value);
                /* free the memory used to store the name and val of var */
                std::free(mem_name);
                std::free(mem_value);
                std::free(mem_space);
            } else if (packet-> compare_strings(val, "set")) {     
                printf("%s has called for db to set a var.\n", s.username);
                char * mem_name = packet-> get_mem_value("var_name", error, 0, true);
                char * mem_value = packet-> get_mem_value("var_value", error, 0, true);
             
                _this-> db_memory-> set_mem_value(mem_name, mem_value, error);        

                printf("var name: %s, var value: %s\n", mem_name, mem_value);
                /* free the memory used to store the name and val of var */
                std::free(mem_name);
                std::free(mem_value);
            } else if (packet-> compare_strings(val, "get")) {
                printf("%s has called for db to get a var.\n", s.username);
                char * mem_name = packet-> get_mem_value("var_name", error, 0, true);
                char * mem_value = _this-> db_memory-> get_mem_value(mem_name, error, 0, true);
  
                _this-> transmit_packet(socket, mem_value, false, error);

                printf("var name: %s, var value: %s\n", mem_name, mem_value);
                /* free the memory used to store the name and val of var */
                std::free(mem_name);
                std::free(mem_value);
            } 


            _this-> db_memory-> dump_stack_memory();
            std::free(val);
            std::free(packet);  
            
        }
        end:
        printf("ending client thread\n");
        session_info.set_mem_value("allowed_access", "no", error);
        lmanager.end_session();
        socket.close();
        std::free(__socket);
       
        //pthread_cancel((* __th)-> native_handle());
        //std::free(*__th);
    }

    void add_connection(boost::asio::ip::tcp::socket * __socket, qgdb_deamon * _this) {
       boost::thread * th;
        th = new boost::thread(boost::bind(&conn_handler::manage_connection, this, &th, __socket, _this));
    }
} ;
}

# endif /*__conn__handler__hpp*/
