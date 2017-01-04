# ifndef __login__manager__hpp
# define __login__manager__hpp
# include <openssl/sha.h>
# include <base64.h>
# define MAX_UNAME_LEN 16
# define MAX_PASSWD_LEN 16
# define DEFAULT_DB_LEN 128
# include <tagged_memory.hpp>
namespace mdl { class login_manager
{
    public:
    login_manager(unsigned int __db_len = DEFAULT_DB_LEN) {
        /* args for the seporators can be left empty as we are going to use the defaults
        */
        static tmem_t _user_db(__db_len, {':', '~', ';'}, true);

        // load it into memory for analyzing 
        _user_db.load_mem_stack_from_file("user_db.qg_db");
        bool error = false;

        _user_db.analyze_stack_memory(error);

        this-> user_db = &_user_db;
    }

    bool check_login_data(char * __uname, char * __passwd, bool & __error) {
        /* check the length of the username and password and check if its over
        * the maxed allowed length or eatch of them. if so then return a error.
        */
        if (std::strlen(__uname) > MAX_UNAME_LEN || 
            std::strlen(__passwd) > MAX_PASSWD_LEN
            || this-> does_uname_exist(__uname, __error) == false) {
            __error = true;
            return false;
        } 
   
        /* this is to make sure that its wiped clean before ever use
        */ 
        std::memset(passwd_hash, '\0', 20);

        // has the password and store it for later
        SHA1(reinterpret_cast<unsigned char *>(__passwd), std::strlen(__passwd), this-> passwd_hash);

        return this-> is_passwd_right(__uname);
    }

    bool does_uname_exist(char * __uname, bool & __error) {
        /* check and find if the username of that persion exists in the database
        */
        bool result = this-> user_db-> does_mem_name_exist(__uname, __error);
            
        /* check the result is false, kill the script and return a error
        */
        if (! result) {
            printf("the username %s does not exists in the user database.\n", __uname);
            __error = true;

            return false;
        }

        return true; 
    }

    bool is_passwd_right(char * __uname) {
        bool error = false;

        /* retreve the password from the database, so we can use it to compare
        * with the existing one we have recived from the user.
        */
        char * passwd = this-> user_db-> get_mem_value(__uname, error); 

        char * base64_str = nullptr;
       
        printf("Hello Testing\n");

        std::size_t b64_len = Base64decode_len(passwd);
        base64_str = static_cast<char *>(malloc(b64_len));
        memset(base64_str, '\0', b64_len);

        Base64decode(base64_str, passwd);

        std::free(passwd); 
    
        passwd = static_cast<char *>(malloc(b64_len));
        memset(passwd, '\0', b64_len);    

        for (std::size_t i = 0; i != b64_len; i ++) {
            printf("setting %c\n", base64_str[i]);
            passwd[i] = base64_str[i]; 
        }
        
        std::free(base64_str);

        // NOTE: i dont know if this might work in the long run
        unsigned char * password = reinterpret_cast<unsigned char *>(passwd);

        /* we need the length of both password length's for later 
        */
        unsigned int passwd_hash_len = std::strlen(reinterpret_cast<char *>(this-> passwd_hash)); 
        unsigned int passwd_db_hash_len = std::strlen(passwd);

/*        
        for (std::size_t i = 0; i != passwd_db_hash_len; i ++) {
            if (passwd[i] == '1') passwd[i] = ';';
            if (passwd[i] == '2') passwd[i] = ':';
            if (passwd[i] == '3') passwd[i] = '~';
            if (passwd[i] == '4') passwd[i] = ',';
        }
*/
        for (std::size_t i = 0; i != passwd_db_hash_len; i ++)
            printf("%u - ", password[i]);

        printf("\n");

        for (std::size_t i = 0; i != passwd_hash_len; i ++)
            printf("%u - ", this-> passwd_hash[i]);


        printf("\n");
        /* if the password hash is not the right length then we know its
        * not going to be the same.
        */
        printf("checking length: %d , %d\n", passwd_hash_len, passwd_db_hash_len);
        if (passwd_hash_len != passwd_db_hash_len) return false;

        /* count the amount of matching charicters in the password hash
        */
        unsigned int match_count = 0; 
        
        /* check each charicter for a match, if we have a match
        * then plus 1 to 'match_count' var.
        */
        for (std::size_t i = 0 ; i != passwd_hash_len ; i ++)  {
            if (this-> passwd_hash[i] == password[i]) match_count ++;
            printf("checking %u against %u\n", passwd_hash[i], password[i]);
        } 
        
        /* if the match count does not equal the char count of the hash then its wrong.
        */
        if (match_count == (passwd_hash_len)) return true;

        return false;
    }

    bool is_logged_in() {
        return this-> current_session.is_logged_in;
    }

    char * get_username() {
        return this-> current_session.username;
    }

    char * get_password() {
        return this-> current_session.password;
    }

    typedef struct {
        /* NOTE: the username and password are pointers
        * the memory will be freed when we call end_sesion.
        */
        /* where we will store the sesion username
        */
        char * username = nullptr;
    
        /* where we will store the sesion password
        */
        char * password = nullptr;
    
        /* are we currently logged in ?
        */
        bool is_logged_in = false;

        /* has the user just logged in ?
        */
        bool just_logged_in = false;
    } session;

    void start_session(char * uname, char * passwd) {
        session & s = this-> current_session;

        /* if the user is allready logged in then we cant login in that user agine. 
        * NOTE: dont remove as it will cause a memory leak.
        */
        if (s.is_logged_in) {
            printf("cant start session as we are allready logged in.\n");
            return;
        } 
        
        /* set the username so we can use it later.
        */
        s.username = uname;
        
        /* set the password, NOTE: password is hashed
        */
        s.password = passwd;
 
        s.just_logged_in = true;
        s.is_logged_in = true; 
    }

    void end_session() {
        session & s = this-> current_session;

        if (s.is_logged_in == true) {
            std::free(s.username);
            std::free(s.password);
            s.is_logged_in = false;
            s.just_logged_in = false;
        }
    }

    session & get_session() {
        return this-> current_session;
    }

    private :
    session current_session;
    tmem_t * user_db = nullptr; 
    unsigned char passwd_hash[20];
    
} ;

}

# endif /*__login__manager__hpp*/
