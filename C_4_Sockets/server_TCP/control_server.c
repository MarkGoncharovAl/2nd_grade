#include "Com_libs/includes.h"
#include "Com_libs/const.h"
#include "packet/packet.h"

in_addr_t check_argc (int argc , char* argv);
int GetID(int sk, struct sockaddr* addr1, struct sockaddr* addr2);

int main (int argc , char* argv [])
{
    if (argc != 2)
    {
        WARNING("Wrong number of arguments!");
        return 0;
    }
    if (strcmp (argv[1], "CLOSE_SERVER") != CMP_EQ)
    {
        WARNING("Useless calling of control_server!");
        return 0;
    }

    struct in_addr in_ad = {inet_addr (INET)};

    int sk = socket (AF_INET , SOCK_STREAM , 0);
    ERROR_CHECK (sk , SOCK_ERR , "Unable to connect to socket!");

    const struct sockaddr_in sending = { AF_INET, PORT_TCP, in_ad, 0 } ;
    struct sockaddr* sending_ = (struct sockaddr*)&sending;

    M_pack_named* pack = M_CreatePack_Named (argv[1] , strlen(argv[1]) , -1);
    M_WritePack_Named (sk , sending_ , pack);
    M_DestroyPack_Named (pack);

    close (sk);
    return 0;
}