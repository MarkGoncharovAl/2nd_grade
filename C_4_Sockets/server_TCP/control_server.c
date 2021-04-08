#include "../Com_libs/includes.h"
#include "../Com_libs/const.h"
#include "../packet/packet.h"

int SetTempLog ();

int main (int argc , char* argv [])
{
    if (SetTempLog () == -1)
        return -1;

    if (argc != 2)
    {
        pr_err ("Wrong number of arguments! Should be 2");
        return 1;
    }
    if (strcmp (argv[1] , "CLOSE_SERVER") != CMP_EQ)
    {
        pr_err ("Useless calling of control_server!");
        return 1;
    }

    struct in_addr in_ad = { inet_addr (INET) };

    int sk = socket (AF_INET , SOCK_DGRAM , 0);
    if (sk == SOCK_ERR)
    {
        pr_err ("Unable to connect to socket!");
        return 1;
    }

    const struct sockaddr_in sending = { AF_INET, PORT, in_ad, 0 };
    struct sockaddr* sending_ = (struct sockaddr*)&sending;

    M_pack_named* pack = M_CreatePack_Named (argv[1] , strlen (argv[1]) , -1);
    if (pack == NULL)
        return 1;

    if (M_WritePack_Named (sk , sending_ , pack) == -1)
        return 1;

    M_DestroyPack_Named (pack);

    close (sk);
    return 0;
}

int SetTempLog ()
{
    int logfd = fast_open ("tmp.log");
    if (logfd == -1)
        return -1;

    if (SetLogFile (logfd))
        return -1;

    return 0;
}