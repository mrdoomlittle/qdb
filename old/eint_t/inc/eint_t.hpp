# ifndef __eint__t__hpp
# define __eint__t__hpp
# include <boost/cstdint.hpp>

namespace mdl {
# ifdef ARC64
    typedef boost::uint64_t uint_t;
    typedef boost::int64_t int_t;
# elif ARC32
    typedef boost::uint32_t uint_t;
    typedef boost::int32_t int_t;
# elif ARC16
    typedef boost::uint16_t uint_t;
    typedef boost::int16_t int_t;
# elif ARC8
    typedef boost::uint8_t uint_t;
    typedef boost::int8_t int_t;
# else
    typedef int unsigned uint_t;
    typedef int int_t;
# endif
}

# endif /*__eint__t__hpp*/
