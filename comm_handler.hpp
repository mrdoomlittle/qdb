# ifndef __comm__handler__hpp
# define __comm__handler__hpp
# include <boost/cstdint.hpp>
# include <boost/cstdlib.hpp>
# include <boost/asio.hpp>
# include <vector>
# include <sstream>
# include <tagged_memory.hpp>

/* length of the buffer in chars/ bytes */
# define BUFFER_LENGTH 2048
# define PK_HEADER_LEN 64

/* NOTE: need to change the naming of this
* as its extra length on top of the body length.
*/
# define PK_BODY_LEN 2048
namespace mdl { class comm_handler
{
    public:
    void send_pk_header(boost::asio::ip::tcp::socket & __socket, std::size_t __body_len, bool & __error, boost::system::error_code & __error_code, bool debug = false) {
        tagged_memory::extra_options_t options;

        /* we are only using this to add and change stuff to make
        * editing the header and what it contains easer.
        */
        tmem_t pk_header(PK_HEADER_LEN, {}, options, debug);

        /* convert the body length(int) to a std string
        * later this will allows us to turn it into a c string.
        */
        std::string body_len = std::to_string(__body_len);

        /* add a tag that will allow the client to know the length
        * of the body.
        */
        pk_header.add_mem_tag("body_length", body_len.c_str(), 0, __error);

        /*
        *
        * NOTE: if anything else needs adding to the heade do it hear.
        *
        */

        /* as tmem_t is not a char type, we need somwhere to dump the stack
        * so we can pass it thru the asio write function.
        */
        char * header_buffer = static_cast<char *>(std::malloc((PK_HEADER_LEN + 1) * sizeof(char)));
        std::memset(header_buffer, '\0', (PK_HEADER_LEN + 1) * sizeof(char));

        /* get the contents of the header memory stack
        */
        char * header_stack = pk_header.dump_stack_memory(true);

        /* put the contents of the header memory stack into the header buffer.
        */
        for (std::size_t i = 0 ; i != PK_HEADER_LEN; i ++) {
            /* check if the array of char's has ended */
            if (header_stack[i] == '\0') break;

            header_buffer[i] = header_stack[i];
        }

        /* write the data to the socket.
        */
        boost::asio::write(__socket, boost::asio::buffer(header_buffer, PK_HEADER_LEN), __error_code);
        if (__error_code == boost::asio::error::eof) __error = true;

        /* free the memory that we used.
        */
        std::free(header_buffer);
        std::free(header_stack);
    }

    tmem_t * recv_pk_header(boost::asio::ip::tcp::socket & __socket, bool & __error, boost::system::error_code & __error_code, bool debug = false) {
        /* create some space where we can store the data we get from the client.
        */
        char * header_buffer = static_cast<char *>(std::malloc((PK_HEADER_LEN + 1) * sizeof(char)));
        std::memset(header_buffer, '\0', (PK_HEADER_LEN + 1) * sizeof(char));

        /* read the socket.
        */
        boost::asio::read(__socket, boost::asio::buffer(header_buffer, PK_HEADER_LEN), __error_code);

        /* check if there's any error's
        */
        if (__error_code == boost::asio::error::eof) {
            std::free(header_buffer);
            __error = true;
            return nullptr;
        }

        tagged_memory::extra_options_t options;

        /* this is where we will store it for interpretation
        */
        tmem_t * pk_header = new tmem_t(PK_HEADER_LEN, {}, options, debug);

        /* dump the data we got into the stack
        */
        pk_header-> dump_into_stack(header_buffer);

        /* analyze the stack.
        */
        pk_header-> analyze_stack_memory(__error);

        /* free any memory we used.
        */
        std::free(header_buffer);

        /* return the header as a tmem_t
        */
        return pk_header;
    }

    void transmit_packet(boost::asio::ip::tcp::socket & __socket, tmem_t * __pk_body, bool debug, bool& __error)
    {
        boost::system::error_code error_code;

        /* dump the packet body stack memory so we can send it using asio::write
        * as it only takes char * or std::vector i think?
        */
        char * pk_body_stack = __pk_body-> dump_stack_memory(true);

        /* store the length of the body as we will be needing it later */
        std::size_t body_stack_len = std::strlen(pk_body_stack);

        /* send the packet header, this will contain the body length and
        * other stuff if needed.
        */
        this-> send_pk_header(__socket, body_stack_len, __error, error_code, debug);
        if (__error) goto end;

        printf("sending packet body: %s\n", pk_body_stack);
        boost::asio::write(__socket, boost::asio::buffer(pk_body_stack, body_stack_len), error_code);
        if (error_code == boost::asio::error::eof) __error = true;

        end:
        std::free(pk_body_stack);
    }

    /* this just allows me to transmit data e.g. transmit_packet(":example~value;");
    */
    void transmit_packet(boost::asio::ip::tcp::socket & __socket, char const * __pk_body, bool debug, bool& __error) {
        boost::system::error_code error_code;

        /* store the length of the body as we will be needing it later */
        std::size_t body_len = std::strlen(__pk_body);

        /* send the packet header, this will contain the body length and
        * other stuff if needed.
        */
        this-> send_pk_header(__socket, body_len, __error, error_code, debug);
        if (__error) return;

        printf("sending packet body: %s\n", __pk_body);
        boost::asio::write(__socket, boost::asio::buffer(__pk_body, body_len), error_code);
        if (error_code == boost::asio::error::eof) __error = true;
    }

    tmem_t * receive_packet(boost::asio::ip::tcp::socket & __socket, bool & __error, bool debug = false)
    {
        boost::system::error_code error_code;

        /* get the packet header as we need to know the length of the body,
        * because it will be needed when receiveing it.
        */
        tmem_t * pk_header = this-> recv_pk_header(__socket, __error, error_code, debug);
        if (__error == true) return nullptr;

        /* extract the body length from the packet header
        */
        std::size_t body_len = atoi(pk_header-> get_mem_value("body_length", null_idc, __error, 0, true));

        tagged_memory::extra_options_t options;

        /* this is where the packet body will be stored when finished
        */
        tmem_t * pk_body = new tmem_t((body_len + PK_BODY_LEN), {}, options, debug);

        /* this is where we will store the packet body when reading the data, after that it will
        * be dumped into the pk_body stack.
        */
        char * body_buffer = static_cast<char *>(malloc((body_len + 1) * sizeof(char)));
        std::memset(body_buffer, '\0', (body_len + 1) * sizeof(char));

        /* receive the packet body */
        boost::asio::read(__socket, boost::asio::buffer(body_buffer, body_len), error_code);
        printf("receiving packet body: %s\n", body_buffer);

        if (error_code == boost::asio::error::eof) {
            __error = true;
            std::free(body_buffer);
            std::free(pk_header);

            return nullptr;
        }

        /* dump the contents of the body buffer into the tmem_t(pk_body)
        */
        pk_body-> dump_into_stack(body_buffer);

        /* analyze the memory stack of the packet body
        */
        pk_body-> analyze_stack_memory(__error);

        /* free the memory we used.
        */
        std::free(body_buffer);
        std::free(pk_header);

        /* return the body
        */
        return pk_body;
    }
} ;
}

# endif /*__comm__handler__hpp*/
