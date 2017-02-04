# include "qgdb_connect.hpp"
# include <chrono>
int main () {
    boost::asio::io_service io_service;
    mdl::connection_info cinfo;

    cinfo._port_number = 21299;
    cinfo._ipv4_address = "192.168.0.10";

    mdl::qgdb_connect qgdb_connect(io_service);

    qgdb_connect.initialize(cinfo);
    bool e = false;
    
    mdl::tmem_t * config = qgdb_connect.receive_config(e);
    config-> analyze_stack_memory(e);


    mdl::tmem_t * session_info = qgdb_connect.recv_session_info(e);

    char * username = "mrdoomlittle";
    char * password = "mello";
    mdl::tagged_memory::extra_options_t options;
    mdl::tmem_t a(BUFFER_LENGTH, {}, options, false);

    qgdb_connect.login(username, password, a);

    std::free(session_info);
 
    session_info = qgdb_connect.recv_session_info(e);
    
    std::free(session_info);

    qgdb_connect.add("count", "NULL", 0, a);

    std::size_t x = 1080;
    char go[1080];

    for (std::size_t i = 0; i != 1080; i ++) {
        go[i] = 'A';
    }

  
    for (std::size_t i = 0; i != 256; i ++) {
        session_info = qgdb_connect.recv_session_info(e);

//        std::string val = std::to_string(i);

        auto begin = std::chrono::high_resolution_clock::now();
        qgdb_connect.set("count", go, a); 
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
        std::cout << "time taken to set mem value: " << time_span.count() << std::endl;
    

        std::free(session_info);
    }
    

 
    std::free(config);
//    std::free(session_info);
}
