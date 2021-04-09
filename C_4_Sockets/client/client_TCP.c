#include "../Com_libs/includes.h"
#include "../Com_libs/const.h"
#include "../packet/packet.h"

static int SetLogFileID ();
static int StartClient (int sk , struct sockaddr* send);

#define LOGGING_FILE "/var/log/clientTCP.log"

int main (int argc , char* argv [])
{
    if (SetLogFileID () == -1)
        return -1;
    pr_info ("Logging is starting");

    struct in_addr in_ad = { inet_addr (INET) };
    if (in_ad.s_addr == -1)
        return -1;

    int sk = socket (AF_INET , SOCK_STREAM , 0);
    if (sk == SOCK_ERR)
    {
        pr_strerr ("Unable to connect to socket!");
        return 1;
    }

    const struct sockaddr_in sending = { AF_INET, PORT, in_ad, 0 };
    struct sockaddr* sending_ = (struct sockaddr*)&sending;

    if (connect (sk , sending_ , sizeof (sending)) == -1)
    {
        pr_strerr ("Unable to connect to socket!");
        return 1;
    }

    pr_info ("Client was initialized");
    StartClient (sk , sending_);

    pr_info ("Unlinking");
    close (sk);
    return 0;
}

int StartClient (int sk , struct sockaddr* send)
{
    M_pack_named* first_pack = M_ReadPack_Named (sk , send);
    if (first_pack == NULL)
        return -1;
    printf ("%s" , first_pack->data_);
    M_DestroyPack_Named (first_pack);

    char getstr[BUFSZ] = {};
    while (1)
    {
        if (fgets (getstr , BUFSZ , stdin) == NULL)
        {
            pr_strerr ("Unable to getstr!");
            return -1;
        }

        size_t len = strlen (getstr);
        getstr[len - 1] = '\0'; //delete last '\n' after fgets

        M_pack_named* pack = M_CreatePack_Named_Mem (getstr , len , 0);
        if (pack == NULL)
            return -1;

        pr_info ("Client's message: %s" , getstr);
        if (M_WritePack_Named (sk , send , pack) == -1)
            return -1;

        free (pack);

        if (strcmp (getstr , "exit") == CMP_EQ || strcmp (getstr , "CLOSE_SERVER") == CMP_EQ)
            return 0;

        M_pack_named* packet = M_ReadPack_Named (sk , send);
        if (pack == NULL)
            return -1;

        printf ("%s" , packet->data_);
        M_DestroyPack_Named (packet);
    }
    return 0;
}

int SetLogFileID ()
{
    int logfd = fast_open (LOGGING_FILE);
    if (logfd == -1)
        return -1;

    if (SetLogFile (logfd))
        return -1;

    pr_info ("Log file was sucesfully set");
    return 0;
}