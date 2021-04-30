#include "../Com_libs/includes.h"
#include "../Com_libs/const.h"
#include "../packet/packet.h"
#include "../ID/ID.h"
#include "server_union.h"

#define NEW_CLIENT -1
static const char LOG_FILE [] = "/var/log/SERVER/server.log";

//return 0 if success
int WriteMessage (int ID , M_pack_unnamed* pack);
int InitDaemon ();
int StartServer (int sk , struct sockaddr_in* name , struct sockaddr* name_);

//-1 - Error was occured
// 0 - Client was already signed
// 1 - New client
char CreateNewClient (struct sockaddr_in* addr , int sk);

void CloseAll (int socket);

int main ()
{
    if (InitDaemon () == -1)
        return EXIT_SUCCESS;

    int sk = socket (AF_INET , SOCK_DGRAM , 0);
    if (sk == SOCK_ERR)
    {
        pr_err ("Unable to create socket!");
        return EXIT_FAILURE;
    }

    struct in_addr in_ad = { inet_addr (INET) };
    struct sockaddr_in name = { AF_INET, PORT, in_ad, 0 };
    struct sockaddr* name_ = (struct sockaddr*)&name;

    if (bind (sk , name_ , sizeof (struct sockaddr_in)) == SOCK_ERR)
    {
        pr_err ("Unable to bind socket!");
        close (sk);
        return EXIT_FAILURE;
    }

    pr_info ("Server was initialized!");
    StartServer (sk , &name , name_);

    pr_info ("Exit programm");
    CloseAll (sk);
    return 0;
}

int StartServer (int sk , struct sockaddr_in* name , struct sockaddr* name_)
{
    M_pack_named* pack = NULL;
    int ret = 0;

    while (1)
    {
        pack = M_ReadPack_Named (sk , name_);

        if (pack == NULL)
            return -1;

#ifdef MAX_INFO
        pr_info ("Readed: %s" , pack->data_);
#endif

        if (strcmp (pack->data_ , "CLOSE_SERVER") == CMP_EQ)
            return 0;

        if (pack->ID_ == NEW_CLIENT)
        {
            ret = CreateNewClient (name , sk);
            if (ret == -1)
                goto exit;
        }
        else //client was already created
        {
            ret = WriteMessage (pack->ID_ , M_RecoverPack (pack));
            if (ret == -1)
                goto exit;
        }
    }

exit:
    M_DestroyPack_Named (pack);
    return ret;
}

char CreateNewClient (struct sockaddr_in* addr , int sk)
{
    int new_pipe[2] = {};
    if (pipe (new_pipe) == -1)
    {
        pr_err ("Can't create pipe");
        return -1;
    }

    int new_id = M_CreateID_FromFD (new_pipe[1]);

    if (fork () == 0)
    {//child
        //!Changed: exec -> function call
        close(new_pipe[1]);
        pr_info("Slave %d has to be started!", new_id);
        StartServerSlave(new_pipe[0], sk, addr->sin_port, addr->sin_addr.s_addr, new_id);
        pr_info("Slave %d has to be dead", new_id);
        raise(SIGKILL);
    }

    pr_info ("Created new client");
    return 0;
}

int WriteMessage (int ID , M_pack_unnamed* pack)
{
    if (strcmp (pack->data_ , "exit") == CMP_EQ)
    {
        pr_info ("Client %d has went out!" , ID);

        char buf [] = "CLOSE_SERVER";
        M_pack_unnamed* packet = M_CreatePack_Unnamed_Mem (buf , strlen (buf));
        if (packet == NULL)
            return -1;

        if (WriteMessage (ID , packet) == -1)
            return -1;

        M_DeleteID (ID);
        free (packet);
        return 0;
    }

    pr_info ("Writting message to slave: %s" , pack->data_);

    int fd = M_GetFD_FromID (ID);
    if (fd == -1)
        return -1;

    if (M_WritePack_Unnamed (fd , pack) == -1)
        return -1;

    return 0;
}

void CloseAll (int socket)
{
    M_Close_IDS ();
    close (socket);

    pr_info ("Unlinked!");
    UnSetLogFile ();
}

int InitDaemon ()
{
    pr_info ("Initializing daemon!");
    pid_t pd = 0;

    if ((pd = fork ()) == -1)
    {
        pr_strerr ("Can't create pid from fork");
        return -1;
    }
    if (pd != 0)
        return -1;

    umask (0);
    pid_t sid = 0;
    if ((sid = setsid ()) == -1)
    {
        pr_strerr ("Can't set sid");
        return -1;
    }

    if (chdir ("/") == -1)
    {
        pr_strerr ("Can't change directory!");
        return -1;
    }

    pr_info ("Daemon was initialized!");

    int logfd = FastOpen (LOG_FILE);
    if (logfd == -1)
        return -1;
    if (SetLogFile (logfd) == -1)
        return -1;

    close (STDIN_FILENO);
    close (STDOUT_FILENO);
    close (STDERR_FILENO);

    return 0;
}