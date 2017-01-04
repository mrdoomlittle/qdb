# include <openssl/sha.h>
# include <cstdio>
# include <cstring>
#include <fstream>
# include <base64.h>
# include <iostream>
int main(int argc, char * argv[])
{
    unsigned char buff[20];
    SHA1(reinterpret_cast<unsigned char *>(argv[1]), std::strlen(argv[1]), buff);
    //printf("hash: %s", reinterpret_cast<char *>(buff)); 
    char * t = reinterpret_cast<char *>(buff);
/*
    for (int i = 0; i != 20; i ++)
    {
        if (t[i] == ';') t[i] = '1';
        if (t[i] == ':') t[i] = '2';
        if (t[i] == '~') t[i] = '3';
        if (t[i] == ',') t[i] = '4';
        printf("%u - ", buff[i]);
    }   
    printf("\n");
*/
    std::size_t buff_len = strlen(t);
    std::size_t encoded_data_length = Base64encode_len(buff_len);

    char * base64_str = static_cast<char *>(malloc(encoded_data_length)); 

    Base64encode(base64_str, t, buff_len);

    std::ofstream myfile;
    myfile.open ("user_db.qg_db");
    myfile << ":";
    myfile << "mrdoomlittle~";
    myfile << base64_str;
    myfile << ";";
    myfile.close();
}
