#include "../Com_libs/includes.h"
#include "../Com_libs/const.h"
#include "../packet/packet.h"
#include "../ID/ID.h"

#define NEW_CLIENT -1
static const char LOG_FILE [] = "/home/mark/VS_prog/HW4/Socket/server.log";

//return 0 if success
int WriteMessage (int ID , M_pack_unnamed* pack);
int init_daemon ();

//-1 - Error was occured
// 0 - Client was already signed
// 1 - New client
char CheckNewClient (M_pack_named* pack , struct sockaddr_in* addr , int sk);

void Close (int socket);

int main ()
{
    if (init_daemon () == -1)
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
    socklen_t sock_len = sizeof (struct sockaddr_in);

    if (bind (sk , name_ , sock_len) == SOCK_ERR)
    {
        pr_err ("Unable to bind socket!");
        close (sk);
        return EXIT_FAILURE;
    }

    pr_info ("Server was initialized!");
    while (1)
    {
        M_pack_named* pack = M_ReadPack_Named (sk , name_);
        if (pack == NULL)
            goto exit_programm;

#ifdef MAX_INFO
        pr_info ("Readed: %s" , pack->data_);
#endif

        if (strcmp (pack->data_ , "CLOSE_SERVER") == CMP_EQ)
            goto exit_programm;

        switch (CheckNewClient (pack , &name , sk))
        {
        case -1: //error
            goto exit_programm;
        case 0: //client was existed
            if (WriteMessage (pack->name_ , M_RecoverPack (pack)) == -1)
                goto exit_programm;
            break;
        case 1: //new client
            break;
        default: //not done
            pr_err ("Can't understand mistake!");
            goto exit_programm;
        }

        M_DestroyPack_Named (pack);
    }

exit_programm:
    pr_info ("Exit programm");
    Close (sk);
    return 0;
}

char CheckNewClient (M_pack_named* pack , struct sockaddr_in* addr , int sk)
{
    pr_info ("Checking new client");
    if (pack->name_ == NEW_CLIENT)
    {
        int new_pipe[2] = {};
        if (pipe (new_pipe) == -1)
        {
            pr_err ("Can't create pipe");
            return -1;
        }

        int new_id = M_AddID (new_pipe[1]);

        char out_str[5][16] = {};
        if (sprintf (out_str[0] , "%d" , new_pipe[0]) <= 0
         || sprintf (out_str[1] , "%d" , sk) <= 0
         || sprintf (out_str[2] , "%d" , addr->sin_port) <= 0
         || sprintf (out_str[3] , "%d" , addr->sin_addr.s_addr) <= 0
         || sprintf (out_str[4] , "%d" , new_id) <= 0)
        {
            pr_err ("Can't create strings for slaves!");
            return -1;
        }

        if (fork () == 0)
        {//child
            if (execlp ("/home/mark/VS_prog/HW4/Socket/build/./server_slave.o" ,
                "/home/mark/VS_prog/HW4/Socket/build/./server_slave.o" ,
                out_str[0] , out_str[1] , out_str[2] , out_str[3] , out_str[4] , NULL) == EXEC_ERR)
            {
                pr_err ("Can't create server_slave!");
                return -1;
            }
        }

        pr_info ("Created new client: %s" , out_str[4]);
        return 1;
    }

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

    pr_info ("Writting message to slave" , pack->data_);

    int fd = M_GetFD_FromID (ID);
    if (fd == -1)
        return -1;

    if (M_WritePack_Unnamed (fd , pack) == -1)
        return -1;

    return 0;
}

void Close (int socket)
{
    M_Close_IDS (socket);
    close (socket);

    pr_info ("Unlinked!");
    UnSetLogFile ();
}















int init_daemon ()
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

    int logfd = fast_open (LOG_FILE);
    if (logfd == -1)
        return -1;
    if (SetLogFile (logfd) == -1)
    {

        return -1;
    }

    close (STDIN_FILENO);
    close (STDOUT_FILENO);
    close (STDERR_FILENO);

    return 0;
}