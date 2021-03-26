#include "../Com_libs/includes.h"
#include "../Com_libs/const.h"
#include "../packet/packet.h"

in_addr_t check_argc (int argc , char* argv);
int GetID (int sk , struct sockaddr* addr1 , struct sockaddr* addr2);
int SetLogFileID (int ID);
int SetTempLog ();

int main (int argc , char* argv [])
{
    if (SetTempLog () == -1)
        return -1;
    pr_info ("Logging is starting");

    struct in_addr in_ad = { check_argc (argc, argv[1]) };

    int sk = socket (AF_INET , SOCK_DGRAM , 0);
    if (sk == SOCK_ERR)
    {
        pr_strerr ("Unable to connect to socket!");
        return 1;
    }

    if (argc == 2 && strcmp (argv[1] , "BROADCAST") == CMP_EQ)
    {
        int yes = 1;
        if (setsockopt (sk , SOL_SOCKET , SO_BROADCAST , &yes , sizeof (yes)) == SOCK_ERR)
        {
            pr_strerr ("Can't optionalized socket!");
            return 1;
        }
    }

    const struct sockaddr_in sending = { AF_INET, PORT, in_ad, 0 };
    struct sockaddr* sending_ = (struct sockaddr*)&sending;

    // in order to get information
    ////////////////////////////////////
    const struct sockaddr_in receiving = { AF_INET, 0, in_ad };
    struct sockaddr* receiving_ = (struct sockaddr*)&receiving;
    socklen_t sock_len = sizeof (struct sockaddr_in);

    if (bind (sk , receiving_ , sock_len) == SOCK_ERR)
    {
        pr_strerr ("Unable to bind socket %d" , socket);
        close (sk);
        return 1;
    }
    ////////////////////////////////////

    int ID_CLIENT = GetID (sk , sending_ , receiving_);
    if (ID_CLIENT == -1)
        return -1;

    if (SetLogFileID (ID_CLIENT) == -1)
        return -1;

    pr_info ("Client was initialized");
    char getstr[BUFSZ] = {};
    while (1)
    {
        if (fgets (getstr , BUFSZ , stdin) == NULL)
        {
            pr_strerr ("Unable to getstr!");
            break;
        }

        size_t len = strlen (getstr);
        getstr[len - 1] = '\0'; //delete last '\n' after fgets

        M_pack_named* pack = M_CreatePack_Named_Mem (getstr , len , ID_CLIENT);
        if (pack == NULL)
            break;

        pr_info ("Client's message: %s", getstr);
        if (M_WritePack_Named (sk , sending_ , pack) == -1)
            break;

        free (pack);

        if (strcmp (getstr , "exit") == 0)
            break;

        M_pack_named* packet = M_ReadPack_Named (sk , receiving_);
        if (pack == NULL)
            break;

        printf ("%s" , packet->data_);
        M_DestroyPack_Named (packet);
    }

    pr_info ("Unlinking");
    close (sk);
    return 0;
}

int GetID (int sk , struct sockaddr* addr1 , struct sockaddr* addr2)
{
    pr_info ("Getting ID");

    char buf [] = "GETID";
    M_pack_named* pack = M_CreatePack_Named_Mem (buf , strlen (buf) , -1);
    if (pack == NULL)
        return -1;

    if (M_WritePack_Named (sk , addr1 , pack) == -1)
        return -1;

    free (pack);

    M_pack_named* packet = M_ReadPack_Named (sk , addr2);
    if (packet == NULL)
        return -1;

    int out = atoi (packet->data_);
    printf ("ID of a client: %d\n" , out);
    M_DestroyPack_Named (packet);

    pr_info ("Getted ID %d" , out);
    return out;
}

in_addr_t check_argc (int argc , char* argv)
{
    pr_info ("Cheking for BROADCASTING: ");

    if (argc == 1)
        return inet_addr (INET);
    if (argc == 2)
    {
        if (strcmp (argv , "BROADCAST") == CMP_EQ)
            return 0;
        else
        {
            pr_err ("Should be BROADCAST!\n");
            exit (EXIT_FAILURE);
        }
    }
    if (argc > 2)
    {
        pr_err ("Should be 0 or 1 argument!\n");
        exit (EXIT_FAILURE);
    }

    return 0;
}

int SetLogFileID (int ID)
{
    pr_info ("Setting log file to %d" , ID);

    char buf[20] = {};
    sprintf (buf , "client%d.log" , ID);

    int logfd = fast_open (buf);
    if (logfd == -1)
        return -1;

    if (SetLogFile (logfd))
        return -1;


    pr_info ("Log file was sucesfully set");
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