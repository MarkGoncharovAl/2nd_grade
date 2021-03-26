#include "Com_libs/includes.h"
#include "Com_libs/const.h"
#include "Com_libs/Errors.h"
#include "packet/packet.h"

#define NOT_FOUND_STR "There is no such command!\n"
#define SMALL_BUFF 64
#define wait_ms 1000

void create_bash ();
void check_bash (int fd);

void check_buffer (char* buffer);
void print_cur_dir ();
void send_message (char* str);
void send_message_bash (char* str);
void do_ls ();

//dynamic output
char* write_into_bash (int fd , M_pack_unnamed* pack , struct pollfd* pollfds);

int pipe_rd = 0;
int my_socket = 0;
struct sockaddr* name = NULL;
char* ID = NULL;

char LOG_FILE[40] = "/tmp/my_server_slave";
int log_file = 0;
#define LOG(str) ERROR_CHECK_file(write(log_file, ID, strlen(ID)), -1, "Can't write to log", log_file); ERROR_CHECK_file(write(log_file, ": ", 2), -1, "Can't write to log", log_file); ERROR_CHECK_file(write(log_file, str, strlen(str)), -1, "Can't write to log", log_file)
#define LOG_PURE(str) ERROR_CHECK_file(write(log_file, str, strlen(str)), -1, "Can't write to log", log_file)
#define LOG_ERROR(str)

int main (int argc , char* argv [])
{
    ID = argv[5];
    strcat (LOG_FILE , ID);
    strcat (LOG_FILE , ".txt\0");

    if ((log_file = open (LOG_FILE , O_RDWR | O_CREAT | O_TRUNC , 0666)) == -1)
        return -1;

    if (argc < 6)
    {
        LOG ("Not enough parameters! - exited");
        return 0;
    }

    pipe_rd = atoi (argv[1]); //reading pipe
    my_socket = atoi (argv[2]);
    int port = atoi (argv[3]);
    struct in_addr addr = { atoi (argv[4]) };
    struct sockaddr_in sock_addr = { AF_INET, port, addr, 0 };
    name = (struct sockaddr*)(&sock_addr);


    send_message (argv[5]); //ID CLIENT
    LOG ("Started slaving\n");

    while (1)
    {
        M_pack_unnamed* pack = M_ReadPack_Unnamed (pipe_rd);
        LOG ("Getted message\n");

        if (strcmp (pack->data_ , "CLOSE_SERVER") == CMP_EQ)
        {
            M_DestroyPack_Unnamed (pack);
            break;
        }

        if (strcmp (pack->data_ , "bash") == CMP_EQ)
        {
            M_DestroyPack_Unnamed (pack);
            create_bash ();
            break;
        }

        check_buffer (pack->data_);
        fflush (stdout);
        M_DestroyPack_Unnamed (pack);
    }

    LOG ("Dying\n");
    close (pipe_rd);
    return 0;
}

void create_bash ()
{
    LOG ("Creating bash!\n");
    fflush (stdout);
    int fd = open ("/dev/ptmx" , O_RDWR | O_NOCTTY);
    grantpt (fd);
    unlockpt (fd);
    char* path = ptsname (fd);

    int resfd = open (path , O_RDWR);

    //delete ECHO flag
    struct termios term;
    term.c_lflag = 0;
    if (tcsetattr (resfd , 0 , &term) == -1)
        LOG_ERROR ("Can't set tc_attr\n");

    int pd = 0;
    if ((pd = fork ()) == SOCK_ERR)
        LOG_ERROR ("Fork problem!\n");

    if (pd == 0)
    {
        if (dup2 (resfd , STDIN_FILENO) == -1)
            LOG_ERROR ("DUP1 problem!\n");
        if (dup2 (resfd , STDOUT_FILENO) == -1)
            LOG_ERROR ("DUP2 problem!\n");
        if (dup2 (resfd , STDERR_FILENO) == -1)
            LOG_ERROR ("DUP3 problem!\n");
        close (fd);
        if (execlp ("/bin/bash" , "/bin/bash" , NULL) == -1)
            LOG_ERROR ("EXEC problem!\n");
    }

    LOG ("Created server_slave\n");
    send_message ("Bash was started!\n");
    check_bash (fd);
    close (fd);
    close (resfd);
}

void check_bash (int fd)
{
    struct pollfd pollfds;
    pollfds.fd = fd;
    pollfds.events = POLLIN;

    M_pack_unnamed* packet = M_CreatePack_Unnamed ("ls" , 2);
    //printf("AAAAA\n");
    char* buf_out = write_into_bash (fd , packet , &pollfds);
    //printf("%s\n", buf_out);
    M_DestroyPack_Unnamed (packet);

    while (1)
    {
        M_pack_unnamed* pack = M_ReadPack_Unnamed (pipe_rd);
        LOG_PURE ("Getted message: ");
        LOG_PURE (pack->data_);
        LOG_PURE ("\n");
        if (strcmp (pack->data_ , "CLOSE_SERVER") == CMP_EQ)
        {
            M_DestroyPack_Unnamed (pack);
            break;
        }

        char* buf = write_into_bash (fd , pack , &pollfds);
        M_DestroyPack_Unnamed (pack);
        LOG ("Writted information!\n");
        //LOG_PURE(buf);
        //LOG_PURE("\n");

        // printf("%s\n", buf);
        send_message_bash (buf);
    }
}

void check_buffer (char* buffer)
{
    // buffer[strlen (buffer) - 1] = '\0';
    int n_found = 1;
    switch (buffer[0])
    {
    case 'c':
        if (buffer[1] == 'd')
        {
            n_found = 0;
            int err = 0;

            if (buffer[2] == ' ')
                err = chdir (buffer + 3);
            else if (buffer[2] == '\0')
                err == chdir ("/");
            else
                err = SOCK_ERR;

            if (err == SOCK_ERR)
            {
                send_message ("Can't do this with directories!\n");
                break;
            }

            print_cur_dir ();
            break;
        }

        break;
    case 'l':

        if (strcmp (buffer , "ls") == CMP_EQ)
        {
            n_found = 0;
            do_ls ();
        }
        break;
    case 'p':
        n_found = strcmp (buffer , "print");
        if (n_found == 0)
            send_message (DUMMY_STR);
        break;
    default:
        break;
    }

    if (n_found)
        send_message (NOT_FOUND_STR);
}

void print_cur_dir ()
{
    char buffer[SMALL_BUFF] = { 0 };
    if (getcwd (buffer , SMALL_BUFF) == NULL)
        send_message ("nothing...\n\0");
    else
    {
        strcat (buffer , "\n\0");
        send_message (buffer);
    }
}

void do_ls ()
{
    char buffer1[BUFSZ];
    if (getcwd (buffer1 , BUFSZ) == NULL)
    {
        LOG_ERROR ("Can't define buffer!\n");
        return;
    }

    int new_pipe[2] = {};
    if (pipe (new_pipe) == -1)
    {
        LOG_ERROR ("Can't create pipe\n");
        return;
    }

    pid_t pd = fork ();
    if (pd == 0)
    {
        dup2 (new_pipe[1] , STDOUT_FILENO);
        execlp ("ls" , "ls" , NULL);
    }
    waitpid (pd , NULL , 0);
    if (write (new_pipe[1] , "\0" , 1) == WRITE_ERR)
    {    
}
    //LOG ("Can't properly write\n");

    char buffer[BUFSZ];
    if (read (new_pipe[0] , buffer , BUFSZ) == READ_ERR)
    {    
}
    //LOG ("Can't properly read form pipe!\n");
    else
    {
        strcat (buffer1 , "\n");
        strcat (buffer , "\0");
        strcat (buffer1 , buffer);
        send_message (buffer1);
    }

    close (new_pipe[0]);
    close (new_pipe[1]);
    LOG ("Ls was done\n");
}

void send_message (char* str)
{
    size_t size = strlen (str);
    //printf("%lu\nA\n", size);
    M_pack_named* packet = M_CreatePack_Named (str , size , 0);
    //printf (":%s:%lu:\n" , packet->data_ , packet->size_);
    M_WritePack_Named (my_socket , name , packet);
    M_DestroyPack_Named (packet);
    LOG ("Sended information!\n");
}
void send_message_bash (char* str)
{
    size_t size = strlen (str);
    //printf("%lu\nA\n", size);
    M_pack_named* packet = M_CreatePack_STATIC (str , size + 1 , 0);
    str[size - 1] = ' ';
    str[size] = '\0';
    //printf (":%s:%lu:\n" , packet->data_ , packet->size_);
    M_WritePack_Named (my_socket , name , packet);
    free (packet);
    LOG ("Sended information bash!\n");
}


char big_buffer[64 * BUFSZ] = {};
char* write_into_bash (int fd , M_pack_unnamed* pack , struct pollfd* pollfds)
{
    memcpy (big_buffer , pack->data_ , pack->size_);
    big_buffer[pack->size_] = '\n';
    big_buffer[pack->size_ + 1] = '\0';

    if (write (fd , big_buffer , pack->size_ + 2) == WRITE_ERR)
        LOG_ERROR ("Error in writing!\n");

    int bytes = 0;

    while (poll (pollfds , 1 , wait_ms) != 0)
    {
        if (pollfds->revents == POLLIN)
        {
            int added = read (fd , big_buffer + bytes , sizeof (big_buffer) - bytes);
            if (added == -1)
                LOG_ERROR ("read was unproperly ended!\n");
            bytes += added;
        }
    }

    big_buffer[bytes] = '\0';
    return big_buffer;
}