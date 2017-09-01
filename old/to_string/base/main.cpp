# include <intlen.hpp>
# include <getdigit.hpp>
# include <cstdio>
constexpr char int_as_char[10] =  {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

int main(int argc, char const * argv []) {
    int x = 21299;
    std::size_t xl = mdl::intlen(x);
    char * str = new char [xl];
    for (std::size_t i = 0; i != xl; i ++)
        str[i] = int_as_char[mdl::getdigit(x, i)];

    printf("%s", str);
}
