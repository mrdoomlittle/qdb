# include "strcmb.hpp"
char *mdl::strcmb(char *__str0, char *__str1, boost::uint8_t __free) {
	uint_t str0_len = strlen(__str0);
	uint_t str1_len = strlen(__str1);

	uint_t new_strlen = strlen(__str0) + strlen(__str1) + 1;

	char *strout = static_cast<char *>(malloc(new_strlen * sizeof(char)));
	memset(strout, '\0', new_strlen * sizeof(char));

	for (std::size_t o = 0; o != str0_len; o ++) strout[o] = __str0[o];

	std::size_t i = 0;
	for (std::size_t o = str0_len; o != str0_len + str1_len; o ++, i ++)
		strout[o] = __str1[i];

	switch(__free) {
		case STRCMB_FREE_BOTH:
			std::free(__str0);
			std::free(__str1);
		break;
		case STRCMB_FREE_FIRST:
			std::free(__str0);
		break;
		case STRCMB_FREE_SECOND:
			std::free(__str1);
		break;
	}

	return strout;
}
