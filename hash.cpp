# include <openssl/sha.h>
# include <cstdio>
# include <cstring>
#include <fstream>
int main(int argc, char * argv[])
{
    unsigned char buff[20];
    SHA1(reinterpret_cast<unsigned char *>(argv[1]), std::strlen(argv[1]), buff);
    //printf("hash: %s", reinterpret_cast<char *>(buff)); 
    char * t = reinterpret_cast<char *>(buff);

    for (int i = 0; i != 20; i ++)
    {
        if (t[i] == ';') t[i] = '1';
        if (t[i] == ':') t[i] = '2';
        if (t[i] == '~') t[i] = '3';
        if (t[i] == ',') t[i] = '4';
        printf("%c - ", t[i]);
    }   
    printf("\n");

    std::ofstream myfile;
    myfile.open ("user_db.qg_db");
    myfile << ":";
    myfile << "mrdoomlittle~";
    myfile << t;
    myfile << ";";
    myfile.close();
}
