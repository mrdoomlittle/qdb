# ifndef __client__config__t__hpp
# define __client__config__t__hpp
# include <serializer.hpp>
namespace mdl { namespace qgdb {
typedef struct {
	char x = 'H';
	void achieve(serializer& __arc) {
		__arc & sizeof(char);
		__arc << x;
	}
} client_config_t;
}
}

# endif /*__client__config__t__hpp*/
