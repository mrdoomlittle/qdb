# include "getdigit.hpp"
boost::uint8_t mdl::getdigit(uint_t __uint, std::size_t __unit) {
    if (__uint < 10) return __uint;

    std::size_t len = intlen(__uint);
    if (__unit > len) return __unit;

    uint_t num_unit = 1;
    for (std::size_t o = 0; o != len-1; o ++) num_unit *= 10;
 
    boost::uint8_t * digit = new boost::uint8_t[len];

    uint_t sv = __uint, l = num_unit;
    for (std::size_t i = 0; i != len; i ++) {
        if (i != 0) sv -= (floor(sv / num_unit) * num_unit);
        digit[i] = (sv/l);
        if (i != 0) num_unit /= 10;
        l /= 10; 
    } 

    return digit[__unit];
}

extern "C" {
    boost::uint8_t getdigit(mdl::uint_t __uint, std::size_t __unit) {
        return mdl::getdigit(__uint, __unit);
    }
}
