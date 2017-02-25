# ifndef __set__info__t__hpp
# define __set__info__t__hpp
# include <eint_t.hpp>
namespace mdl { namespace qgdb {
typedef struct {
	uint_t mem_namelen, mem_valuelen;
	void achieve(serializer& __arc) {
		__arc & sizeof(uint_t);
		__arc << mem_namelen;

		__arc & sizeof(uint_t);
		__arc << mem_valuelen;
	}
} set_info_t;

}
}

# endif /*__set__info__t__hpp*/
