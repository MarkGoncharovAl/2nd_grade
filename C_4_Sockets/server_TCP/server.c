#include "../Com_libs/includes.h"
#include "../Com_libs/const.h"
#include "../packet/packet.h"
#include "../ID/ID.h"

static const char LOG_FILE [] = "/var/log/serverTCP.log";
static const int SIZE_CONNECTION = 20;

static int init_daemon ();
static int StartServer (int sk , struct sockaddr_in* name);

static int CreateNewClient (int client_sk , struct sockaddr_in* addr);
static void CloseServer (int sk);

int main ()
{
    if (init_daemon () == -1)
        return EXIT_SUCCESS;

    int sk = socket (AF_INET , SOCK_STREAM , 0);
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
    StartServer (sk , &name);

    pr_info ("Exit programm");
    CloseServer (sk);
    return 0;
}

int StartServer (int sk , struct sockaddr_in* name)
{
    if (listen (sk , SIZE_CONNECTION) == -1)
    {
        pr_strerr ("Can't listen to sockets");
        return -1;
    }

    while (1)
    {
        int client_sk = accept (sk , NULL , NULL);
        if (client_sk < 0)
        {
            pr_strerr ("Can't accept client!");
            return -1;
        }

        if (CreateNewClient (client_sk , name) == -1)
            return -1;

        close (client_sk);
    }
}

int CreateNewClient (int client_sk , struct sockaddr_in* addr)
{
    pr_info ("Creating new client at %d fd" , client_sk);

    int cur_pid = getpid ();
    pr_info ("Current pid is %d" , cur_pid);

    if (fork () == 0)
    {//child
        char out_str[4][16] = {};
        if (sprintf (out_str[0] , "%d" , client_sk) <= 0
        || sprintf (out_str[1] , "%d" , addr->sin_port) <= 0
        || sprintf (out_str[2] , "%d" , addr->sin_addr.s_addr) <= 0
        || sprintf (out_str[3] , "%d" , cur_pid) <= 0)
        {
            pr_err ("Can't create strings for slaves!");
            return -1;
        }

        if (execlp ("/home/mark/VS_prog/2nd_grade/C_4_Sockets/build/./server_slaveTCP.o" ,
            "/home/mark/VS_prog/2nd_grade/C_4_Sockets/build/./server_slaveTCP.o" ,
            out_str[0] , out_str[1] , out_str[2] , out_str[3] , NULL) == EXEC_ERR)
        {
            pr_strerr ("Can't create server_slave!");
            return -1;
        }
    }

    return 0;
}


void CloseServer (int sk)
{
    close (sk);
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