# include "to_string.hpp"

constexpr char int_as_char[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

char *mdl::to_string(uint_t __uint) {
	std::size_t len_of_int = intlen(__uint);
	char *int_as_str = static_cast<char *>(malloc((len_of_int + 1) * sizeof(char)));
	memset(int_as_str, '\0', (len_of_int + 1) * sizeof(char));

	for (std::size_t o = 0; o != len_of_int; o ++) int_as_str[o] = int_as_char[getdigit(__uint, o)];

	return int_as_str;
}
