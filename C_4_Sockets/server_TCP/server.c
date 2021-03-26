#include "Com_libs/includes.h"
#include "Com_libs/const.h"
#include "packet/packet.h"
#include "ID/ID.h"

int log_file = 0;

#define NEW_CLIENT -1
#define LOG_FILE "/tmp/my_server.txt"
#define LOG_FILE_SLAVE "/tmp/my_server_slave.txt"
#define LOG(str) ERROR_CHECK_file(write(log_file, str, strlen(str)), -1, "Can't write to log", log_file)

void WriteMessage (int ID , M_pack_unnamed* pack);
void init_daemon ();

//returns true if confirmed new
char CheckNewClient (M_pack_named* pack , struct sockaddr_in* addr , int sk);
void Close (int socket);

int main ()
{
    int sk = socket (AF_INET , SOCK_STREAM , 0);
    ERROR_CHECK (sk , SOCK_ERR , "Unable to create socket!");

    struct in_addr in_ad = { inet_addr (INET) };
    struct sockaddr_in name = { AF_INET, PORT_TCP, in_ad, 0 };

    struct sockaddr* name_ = (struct sockaddr*)&name;
    socklen_t sock_len = sizeof (struct sockaddr_in);

    ERROR_CHECK_2 (bind (sk , name_ , sock_len) , SOCK_ERR ,
                   "Unable to bind socket!" , close (sk));

    if (listen (sk , MAX_CLIENTS) != 0)
        ERROR ("Can't listen to sockets!");

    init_daemon ();

    while (1)    
{
        M_pack_named* pack = M_ReadPack_Named (sk , name_);

        if (pack == NULL)
            break;

        LOG ("Readed: ");
        LOG (pack->data_);
        LOG ("\n");

        //printf("%s\n", pack->data_);
        if (strcmp (pack->data_ , "CLOSE_SERVER") == CMP_EQ)
            break;

        if (!CheckNewClient (pack , &name , sk))
            WriteMessage (pack->name_ , M_RecoverPack (pack));

        M_DestroyPack_Named (pack);
    }

    Close (sk);
    return 0;
}

char CheckNewClient (M_pack_named* pack , struct sockaddr_in* addr , int sk)
{
    if (pack->name_ == NEW_CLIENT)
    {
        int new_pipe[2] = {};
        if (pipe (new_pipe) == -1)
            LOG ("Can't create pipe");
        int new_id = M_AddID (new_pipe[1]);

        char out_str[5][16] = {};
        sprintf (out_str[0] , "%d" , new_pipe[0]);
        sprintf (out_str[1] , "%d" , sk);
        sprintf (out_str[2] , "%d" , addr->sin_port);
        sprintf (out_str[3] , "%d" , addr->sin_addr.s_addr);
        sprintf (out_str[4] , "%d" , new_id);

        pid_t pd = fork ();
        if (pd == 0
        && execlp ("/home/mark/VS_prog/HW4/Sockets/Project/build/./server_slave.o" , "/home/mark/VS_prog/HW4/Sockets/Project/build/./server_slave.o" , out_str[0] , out_str[1] , out_str[2] , out_str[3] , out_str[4] , NULL) == EXEC_ERR)
        {
            LOG ("Can't create server_slave!");
            raise (SIGKILL);
        }

        LOG ("Created new client: ");
        LOG (out_str[4]);
        LOG ("\n");
        return 1;
    }
    return 0;
}

void Close (int socket)
{
    M_Close_IDS (socket);
    close (socket);
    LOG ("Unlinked!\n");
    close (log_file);
}

void WriteMessage (int ID , M_pack_unnamed* pack)
{
    if (strcmp (pack->data_ , "exit") == CMP_EQ)
    {
        LOG ("Client has exited!\n");
        char buf [] = "CLOSE_SERVER";
        M_pack_unnamed* packet = M_CreatePack_Unnamed (buf , strlen (buf));
        WriteMessage (ID , packet);
        M_DestroyPack_Unnamed (packet);

        M_DeleteID (ID);
        return;
    }

    int fd = M_GetFD_FromID (ID);
    if (fd == -1)
    {
        WARNING ("Can't write into -1 pipe!");
        return;
    }

    LOG ("Writing to slave: ");
    LOG (pack->data_);
    LOG ("\n");
    M_WritePack_Unnamed (fd , pack);
}


void init_daemon ()
{
    pid_t pd = 0;
    ERROR_CHECK ((pd = fork ()) , -1 , "Can't create pif from fork");
    if (pd != 0)
        exit (EXIT_SUCCESS);

    umask (0);
    pid_t sid = 0;
    ERROR_CHECK ((sid = setsid ()) , -1 , "Can't set sid");

    ERROR_CHECK (chdir ("/") , -1 , "Can't change directory!");

    pid_t daemon_pid = getpid ();

    ERROR_CHECK ((log_file = open (LOG_FILE , O_RDWR | O_CREAT | O_TRUNC , 0666)) , -1 , "Can't create log file!");
    char buf[20] = { 0 };
    sprintf (buf , "%d\n" , daemon_pid);
    LOG ("Daemon was initialized!\n");

    close (STDIN_FILENO);
    close (STDOUT_FILENO);
    close (STDERR_FILENO);


    LOG (buf);
}