# ifndef __comm__handler__hpp
# define __comm__handler__hpp
# include <boost/cstdint.hpp>
# include <boost/cstdlib.hpp>
# include <boost/asio.hpp>
# include <vector>
#include <sstream>
# include <tagged_memory.hpp>
/* length of the buffer in chars/ bytes */
# define PACKET_HEAD_LEN 1028
# define BUFFER_LENGTH 2048
# define PK_HEADER_LEN 1024
# define PK_BODY_LEN 1024
namespace mdl { class comm_handler
{
    public:
    void send_pk_header(boost::asio::ip::tcp::socket & __socket, std::size_t __body_len, bool & __error) {
        tmem_t pk_header(PK_HEADER_LEN, {':', ';', '~'}, true);
     
        std::string body_len = std::to_string(__body_len);

        pk_header.add_mem_tag("body_length", body_len.c_str(), 0, __error); 

        char * header_buffer = static_cast<char *>(std::malloc((PK_HEADER_LEN + 1) * sizeof(char)));
        std::memset(header_buffer, '\0', (PK_HEADER_LEN + 1) * sizeof(char));

        char * header_stack = pk_header.dump_stack_memory();

        for (std::size_t i = 0 ; i != PK_HEADER_LEN; i ++) {
            if (header_stack[i] == '\0') break;
            header_buffer[i] = header_stack[i];
        }

        boost::asio::write(__socket, boost::asio::buffer(header_buffer, PK_HEADER_LEN));

        std::free(header_buffer);
        std::free(header_stack);
    }

    tmem_t * recv_pk_header(boost::asio::ip::tcp::socket & __socket, bool & __error, boost::system::error_code & __error_code) {
        char * header_buffer = static_cast<char *>(std::malloc((PK_HEADER_LEN + 1) * sizeof(char)));
        std::memset(header_buffer, '\0', (PK_HEADER_LEN + 1) * sizeof(char));

        boost::asio::read(__socket, boost::asio::buffer(header_buffer, PK_HEADER_LEN), __error_code);

        tmem_t * pk_header = new tmem_t(PK_HEADER_LEN, {':', ';', '~'}, true);

        pk_header-> dump_into_stack(header_buffer);

        pk_header-> analyze_stack_memory(__error);

        std::free(header_buffer);

        return pk_header;
    }

    void transmit_packet(boost::asio::ip::tcp::socket & __socket, tmem_t * __pk_body)
    {
        bool error = false;

        /* dump the packet body stack memory so we can send it using asio::write
        * as it only takes char * or std::vector i think?
        */     
        char * pk_body_stack = __pk_body-> dump_stack_memory();

        /* store the length of the body as we will be needing it later */
        std::size_t body_stack_len = std::strlen(pk_body_stack);

        /* send the packet header, this will contain the body length and
        * other stuff if needed.
        */
        this-> send_pk_header(__socket, body_stack_len, error);
        
        printf("sending packet body: %s\n", pk_body_stack);
        boost::asio::write(__socket, boost::asio::buffer(pk_body_stack, body_stack_len));

        std::free(pk_body_stack);
    }

    /* this just allows me to transmit data e.g. transmit_packet(":example~value;");
    */
    void transmit_packet(boost::asio::ip::tcp::socket & __socket, char const * __pk_body) {
        bool error = false;     

        /* store the length of the body as we will be needing it later */
        std::size_t body_len = std::strlen(__pk_body);

        /* send the packet header, this will contain the body length and
        * other stuff if needed.
        */
        this-> send_pk_header(__socket, body_len, error);

        printf("sending packet body: %s\n", __pk_body);
        boost::asio::write(__socket, boost::asio::buffer(__pk_body, body_len));
    }

    tmem_t * receive_packet(boost::asio::ip::tcp::socket & __socket, bool & __error)
    { 
        boost::system::error_code error_code;

        /* get the packet header as we need to know the length of the body,
        * because it will be needed when receiveing it.
        */
        tmem_t * pk_header = this-> recv_pk_header(__socket, __error, error_code);
    
        /* extract the body length from the packet header
        */
        std::size_t body_len = atoi(pk_header-> get_mem_value("body_length", __error, 0, true));
         
        /* this is where the packet body will be stored when finished
        */
        tmem_t * pk_body = new tmem_t(PK_BODY_LEN, {':', ';', '~'}, true);
    
        /* this is where we will store the packet body when reading the data, after that it will
        * be dumped into the pk_body stack.
        */
        char * body_buffer = static_cast<char *>(malloc((body_len + 1) * sizeof(char)));
        std::memset(body_buffer, '\0', (body_len + 1) * sizeof(char)); 

        /* receive the packet body */
        boost::asio::read(__socket, boost::asio::buffer(body_buffer, body_len), error_code);
        printf("receiving packet body: %s\n", body_buffer);

        /* dump the contents of the body buffer into the tmem_t(pk_body)
        */
        pk_body-> dump_into_stack(body_buffer);

        /* analyze the memory stack of the packet body
        */
        pk_body-> analyze_stack_memory(__error);

        std::free(body_buffer);
        std::free(pk_header); 

        if (error_code == boost::asio::error::eof) { 
            __error = true; 
            return nullptr; 
        }
        return pk_body;
    }
} ;
}

# endif /*__comm__handler__hpp*/
