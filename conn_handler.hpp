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

        login_manager::session & curr_session = lmanager.get_session();

        tagged_memory::extra_options_t extra_options;

        bool error = false;

		// NOTE: need to change this
        tmem_t session_info(128, {}, extra_options, true);

        /* NOTE: fore some resion add_mem_tag dose not work
        * before analyzeing.
        */
        session_info.dump_into_stack("{allowed_access;no}");
        session_info.analyze_stack_memory(error);

        _this-> send_client_config(socket, error);

        if (error == true) goto end;

        while (true) {
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

			session_info.dump_stack_memory();

            char * username;
            char * password;

            if (!curr_session.is_logged_in)
            {
                username = packet-> get_mem_value("username", error, 0, true);
                password = packet-> get_mem_value("password", error, 0, true);

                printf("checking username and password for user.\n");
                if (lmanager.check_login_data(username, password, error))
                    lmanager.start_session(username, password);

                std::cout << "--uname: " << username << ", --passwd: " << password << std::endl;

                if (curr_session.is_logged_in)
                    session_info.set_mem_value("allowed_access", "yes", null_idc, error);

                printf("password was %s\n", curr_session.is_logged_in == false? "incorrect, access not granted" : "correct, access was granted");
           }

            if (!curr_session.is_logged_in || curr_session.just_logged_in) {
                if (!curr_session.is_logged_in) {
                    std::free(username);
                    std::free(password);
                }

                std::free(packet);

                if (curr_session.just_logged_in)
                    curr_session.just_logged_in = false;

                continue;
            }

            printf("user %s has logged into the database.\n", curr_session.username);

            char * op_name = packet-> get_mem_value("tm", error, 0, true);

            printf("operation name: %s\n", op_name);

            if (packet-> compare_strings(op_name, "add")) {
                printf("%s has called for the db to add a pice of memory.\n", curr_session.username);
                /* get the memory name and its value, as we need it. */
                char * mem_name = packet-> get_mem_value("var_name", error, 0, true);
                char * mem_value = packet-> get_mem_value("var_value", error, 0, true);
                char * mem_space = packet-> get_mem_value("var_space", error, 0, true);
                /* add a memory tag to the stack. */
                _this-> db_memory-> add_mem_tag(mem_name, mem_value, atoi(mem_space), error);

                printf("mem name: %s, mem value: %s\n", mem_name, mem_value);
                /* free the memory used to store the name and val of var */
                std::free(mem_name);
                std::free(mem_value);
                std::free(mem_space);
            } else if (packet-> compare_strings(op_name, "set")) {
                printf("%s has called for the db to set a pice of memory.\n", curr_session.username);
                char * mem_name = packet-> get_mem_value("var_name", error, 0, true);
                char * mem_value = packet-> get_mem_value("var_value", error, 0, true);

                _this-> db_memory-> set_mem_value(mem_name, mem_value, null_idc, error);

                printf("mem name: %s, mem value: %s\n", mem_name, mem_value);
                /* free the memory used to store the name and val of var */
                std::free(mem_name);
                std::free(mem_value);
            } else if (packet-> compare_strings(op_name, "get")) {
                printf("%s has called for the db to get a pice of memory.\n", curr_session.username);
                char * mem_name = packet-> get_mem_value("var_name", error, 0, true);
                char * mem_value = _this-> db_memory-> get_mem_value(mem_name, error, 0, true);

                _this-> transmit_packet(socket, mem_value, false, error);

                printf("mem name: %s, mem value: %s\n", mem_name, mem_value);
                /* free the memory used to store the name and val of var */
                std::free(mem_name);
                std::free(mem_value);
            }


            _this-> db_memory-> dump_stack_memory();
            std::free(op_name);
            std::free(packet);

        }
        end:

        printf("ending client thread\n");

        session_info.set_mem_value("allowed_access", "no", null_idc, error);

        lmanager.end_session();

        socket.close();

        std::free(__socket);
    }

    void add_connection(boost::asio::ip::tcp::socket * __socket, qgdb_deamon * _this) {
    	boost::thread * th;
    	th = new boost::thread(boost::bind(&conn_handler::manage_connection, this, &th, __socket, _this));
    }
} ;
}

# endif /*__conn__handler__hpp*/
