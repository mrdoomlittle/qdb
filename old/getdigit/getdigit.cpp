# include <getdigit.hpp>
# include <cstdio>
int main(int argc, char const * argv[])
{
    if (argc < 3) {
        printf("error: needs 2 args.\n");
    }

    printf("%d\n", mdl::getdigit(atoi(argv[1]), atoi(argv[2])));    
}
