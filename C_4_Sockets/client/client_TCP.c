#include "Com_libs/includes.h"
#include "Com_libs/const.h"
#include "packet/packet.h"

in_addr_t check_argc (int argc , char* argv);
int GetID (int sk , struct sockaddr* addr1 , struct sockaddr* addr2);

int main (int argc , char* argv [])
{
    printf (ANSI_COLOR_RED "");
    struct in_addr in_ad = { check_argc (argc, argv[1]) };

    int sk = socket (AF_INET , SOCK_STREAM , 0);
    ERROR_CHECK (sk , SOCK_ERR , "Unable to connect to socket!");

    if (argc == 2 && strcmp (argv[1] , "BROADCAST") == CMP_EQ)
    {
        int yes = 1;
        ERROR_CHECK (setsockopt (sk , SOL_SOCKET , SO_BROADCAST , &yes , sizeof (yes)) , SOCK_ERR ,
        "Can't optionalized socket!");
    }

    const struct sockaddr_in sending = { AF_INET, PORT_TCP, in_ad, 0 };
    struct sockaddr* sending_ = (struct sockaddr*)&sending;

    // in order to get information
    ////////////////////////////////////
    socklen_t sock_len = sizeof (struct sockaddr_in);

    ERROR_CHECK_2 (bind (sk , sending_ , sock_len) , SOCK_ERR ,
                   "Unable to bind socket!" , close (sk));

    ERROR_CHECK(connect(sk, sending_, sock_len), -1, "Connection failed");
    ////////////////////////////////////

    char getstr[BUFSZ] = {};
    while (1)
    {
        ERROR_CHECK (fgets (getstr , BUFSZ , stdin) , NULL , "Unable to getstr!");
        size_t len = strlen (getstr);
        getstr[len - 1] = '\0'; //delete last '\n' after fgets

        //printf("%s\n", getstr);
        M_pack_named* pack = M_CreatePack_Named (getstr , len , 0);
        M_WritePack_Named (sk , sending_ , pack);
        M_DestroyPack_Named (pack);

        if (strcmp (getstr , "exit") == 0)
            break;

        M_pack_named* packet = M_ReadPack_Named (sk , sending_);
        printf (ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RED , packet->data_);
        M_DestroyPack_Named (packet);
    }

    close (sk);
    return 0;
}

int GetID (int sk , struct sockaddr* addr1 , struct sockaddr* addr2)
{
    char buf [] = "GETID";
    M_pack_named* pack = M_CreatePack_Named (buf , strlen (buf) , -1);
    M_WritePack_Named (sk , addr1 , pack);
    M_DestroyPack_Named (pack);

    M_pack_named* packet = M_ReadPack_Named (sk , addr2);
    int out = atoi (packet->data_);
    printf ("ID of a client: %d\n" , out);
    M_DestroyPack_Named (packet);
    return out;
}

in_addr_t check_argc (int argc , char* argv)
{
    if (argc == 1)
        return inet_addr (INET);
    if (argc == 2)
    {
        if (strcmp (argv , "BROADCAST") == CMP_EQ)
            return 0;
        else
        {
            ERROR ("Should be BROADCAST!\n");
        }
    }
    if (argc > 2)
        ERROR ("Should be 0 or 1 argument!\n");

    return 0;
}