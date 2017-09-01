# include "src/to_string.hpp"
# include <cstdio>
int main() {
	int unsigned x = 21299;
	char *t = mdl::to_string(x);
	printf("%s\n", t);
	std::free(t);
}
