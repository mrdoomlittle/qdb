# ifndef __strcmb__hpp
# define __strcmb__hpp
# include <boost/cstdint.hpp>
# include <eint_t.hpp>
# include <string.h>
# define STRCMB_FREE_NONE 0
# define STRCMB_FREE_BOTH 1
# define STRCMB_FREE_FIRST 2
# define STRCMB_FREE_SECOND 3
namespace mdl {
char* strcmb(char *__str0, char *__str1, boost::uint8_t __free);
}

# endif /*__strcmb__hpp*/
