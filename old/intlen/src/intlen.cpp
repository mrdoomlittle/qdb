# include "intlen.hpp"

std::size_t mdl::intlen(uint_t __uint)
{
    std::size_t len_of_int = 0;
    uint_t base_unit = 10;
    
    for (uint_t i = base_unit;; i *= base_unit) {
        if (i <= __uint) {
                if (i == base_unit)
                    len_of_int = 2;
                else len_of_int ++;
        } else break;
    }

    if (len_of_int == 0) len_of_int = 1;

    return len_of_int;
}

extern "C" {
    std::size_t intlen(mdl::uint_t __uint) {
        return mdl::intlen(__uint);
    }
}

