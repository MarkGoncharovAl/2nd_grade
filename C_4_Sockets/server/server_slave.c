#include "../Com_libs/includes.h"
#include "../Com_libs/const.h"
#include "../packet/packet.h"

#define NOT_FOUND_STR "There is no such command!\n"
#define SMALL_BUFF 64
#define wait_ms 1000

int create_bash ();
int check_bash (int fd);

int check_buffer (char* buffer);
int print_cur_dir ();
int send_message (char* str);
int do_ls ();

//dynamic output
char* write_into_bash (int fd , M_pack_unnamed* pack , struct pollfd* pollfds);

int SetLogFileID (char* ID);

static int pipe_rd = 0;
static int my_socket = 0;
struct sockaddr* name = NULL;
char* ID = NULL;

/* argv:
[1] - pipe_reading
[2] - socket sending
[3] - sin_port
[4] - sin_addr
[5] - ID
*/

int main (int argc , char* argv [])
{
    if (SetLogFileID (argv[5]) == -1)
        return -1;

    pr_info ("Starting logging!");

    if (argc != 6)
    {
        pr_err ("Not enough parameters: %d. Must be 6" , argc);
        return 0;
    }

    pipe_rd = atoi (argv[1]);
    my_socket = atoi (argv[2]);
    int port = atoi (argv[3]);
    struct in_addr addr = { atoi (argv[4]) };
    struct sockaddr_in sock_addr = { AF_INET, port, addr, 0 };
    name = (struct sockaddr*)(&sock_addr);

    if (send_message (argv[5]) == -1) //ID CLIENT
        return -1;

    pr_info ("Server slave was initialized");

    while (1)
    {
        M_pack_unnamed* pack = M_ReadPack_Unnamed (pipe_rd);
        if (pack == NULL)
            goto exit_programm;

        if (strcmp (pack->data_ , "CLOSE_SERVER") == CMP_EQ)
        {
            M_DestroyPack_Unnamed (pack);
            goto exit_programm;
        }

        if (strcmp (pack->data_ , "bash") == CMP_EQ)
        {
            M_DestroyPack_Unnamed (pack);
            create_bash ();
            goto exit_programm;
        }

        if (check_buffer (pack->data_) == -1)
            goto exit_programm;

        M_DestroyPack_Unnamed (pack);
    }

exit_programm:
    pr_info ("Exit programm server_slave")
        close (pipe_rd);
    return 0;
}

int SetLogFileID (char* ID)
{
    char buf[100] = {};
    if (sprintf (buf , "/home/mark/VS_prog/HW4/Socket/slave%s.log" , ID) == -1)
    {
        pr_strerr ("Can't create name of log file slave%s" , ID);
        return -1;
    }

    int logfd = fast_open (buf);
    if (logfd == -1)
        return -1;
    if (SetLogFile (logfd) == -1)
        return -1;
    return 0;
}

int create_bash ()
{
    pr_info ("Creating bash!");

    int fd = open ("/dev/ptmx" , O_RDWR | O_NOCTTY);
    if (fd == -1)
    {
        pr_strerr ("Can't open /dev/ptmx");
        return -1;
    }

    if (grantpt (fd) == -1)
    {
        pr_strerr ("Can't grantpt bash of %d" , fd);
        return -1;
    }

    if (unlockpt (fd) == -1)
    {
        pr_strerr ("Can't unlock %d" , fd);
        return -1;
    }

    char* path = ptsname (fd);
    if (path == NULL)
    {
        pr_strerr ("Can't set ptsname of %d" , fd);
        return -1;
    }

    int resfd = open (path , O_RDWR);
    if (resfd == -1)
    {
        pr_strerr ("Can't open %s" , path);
        return -1;
    }

    //delete ECHO flag
    struct termios term;
    term.c_lflag = 0;
    if (tcsetattr (resfd , 0 , &term) == -1)
    {
        pr_strerr ("Can't set tc_attr in %d fd" , resfd);
        return -1;
    }

    if (fork () == 0)
    {
        if (dup2 (resfd , STDIN_FILENO) == -1
         || dup2 (resfd , STDOUT_FILENO) == -1
         || dup2 (resfd , STDERR_FILENO) == -1)
        {
            pr_strerr ("dup problem with %d fd" , resfd);
            return -1;
        }

        close (fd);
        if (execlp ("/bin/bash" , "/bin/bash" , NULL) == -1)
        {
            pr_strerr ("exec bash problem!");
            return -1;
        }
    }

    pr_info ("Created bash of server slave");
    if (send_message ("Bash was started!\n") == -1)
        return -1;

    check_bash (fd);
    close (fd);
    close (resfd);
    return 0;
}

int check_bash (int fd)
{
    pr_info ("Checking bash");
    struct pollfd pollfds;
    pollfds.fd = fd;
    pollfds.events = POLLIN;

    M_pack_unnamed* packet = M_CreatePack_Unnamed ("ls" , 2);
    if (packet == NULL)
        return -1;

    char* buf_out = write_into_bash (fd , packet , &pollfds);
    if (buf_out == NULL)
        return -1;

    M_DestroyPack_Unnamed (packet);

    while (1)
    {
        M_pack_unnamed* pack = M_ReadPack_Unnamed (pipe_rd);
        if (pack == NULL)
            return -1;

        pr_info ("Getted message: %s" , pack->data_);
        if (strcmp (pack->data_ , "CLOSE_SERVER") == CMP_EQ)
        {
            M_DestroyPack_Unnamed (pack);
            return -1;
        }

        char* buf = write_into_bash (fd , pack , &pollfds);
        if (buf == NULL)
        {
            M_DestroyPack_Unnamed (pack);
            return -1;
        }

        M_DestroyPack_Unnamed (pack);
        pr_info ("SENDING: %s", buf);
        if (send_message (buf) == -1)
            return -1;
    }

    return 0;
}

int check_buffer (char* buffer)
{
    pr_info ("Checking buffer: %s" , buffer);

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
                if (send_message ("Can't do this with directories!\n") == -1)
                    return -1;
                break;
            }

            if (print_cur_dir () == -1)
                return -1;
            break;
        }

        break;
    case 'l':
        if (strcmp (buffer , "ls") == CMP_EQ)
        {
            n_found = 0;
            if (do_ls () == -1)
                return -1;
        }
        break;
    case 'p':
        n_found = strcmp (buffer , "print");
        if (n_found == 0)
            if (send_message (DUMMY_STR) == -1)
                return -1;
        break;
    default:
        break;
    }

    if (n_found)
        if (send_message (NOT_FOUND_STR) == -1)
            return -1;

    return 0;
}

int print_cur_dir ()
{
    char buffer[SMALL_BUFF] = { 0 };
    if (getcwd (buffer , SMALL_BUFF) == NULL)
    {
        if (send_message ("nothing...\n\0") == -1)
            return -1;
    }
    else
    {
        strcat (buffer , "\n\0");
        if (send_message (buffer) == -1)
            return -1;
    }
    return 0;
}

int do_ls ()
{
    pr_info ("Doing ls");

    char buffer1[BUFSZ];
    if (getcwd (buffer1 , BUFSZ) == NULL)
    {
        pr_strerr ("Can't define buffer for directory!\n");
        return -1;
    }

    int new_pipe[2] = {};
    if (pipe (new_pipe) == -1)
    {
        pr_strerr ("Can't create pipe\n");
        return -1;
    }

    pid_t pd = fork ();
    if (pd == 0)
    {
        if (dup2 (new_pipe[1] , STDOUT_FILENO) == -1)
        {
            pr_strerr ("Can't dup file %d" , new_pipe[1]);
            return -1;
        }
        if (execlp ("ls" , "ls" , NULL) == -1)
        {
            pr_strerr ("Exec ls wasn't done properly!");
            return -1;
        }
        return -1;
    }

    waitpid (pd , NULL , 0);
    if (write (new_pipe[1] , "\0" , 1) == WRITE_ERR)
    {
        pr_strerr ("Can't write into pipe %d" , new_pipe[1]);
        return -1;
    }

    char buffer[BUFSZ];
    if (read (new_pipe[0] , buffer , BUFSZ) == READ_ERR)
    {
        pr_strerr ("Can't read from pipe %d" , new_pipe[1]);
        return -1;
    }
    else
    {
        strcat (buffer1 , "\n");
        strcat (buffer , "\0");
        strcat (buffer1 , buffer);
        if (send_message (buffer1) == -1)
            return -1;
    }

    close (new_pipe[0]);
    close (new_pipe[1]);
    pr_info ("Ls was done");
    return 0;
}

int send_message (char* str)
{
    pr_info ("Sending message");
    size_t size = strlen (str);
    M_pack_named* packet = M_CreatePack_Named_Mem (str , size , 0);
    if (packet == NULL)
        return -1;

    if (M_WritePack_Named (my_socket , name , packet) == -1)
        return -1;

    free (packet);
    pr_info ("Message was sent");
    return 0;
}


char big_buffer[64 * BUFSZ] = {};
char* write_into_bash (int fd , M_pack_unnamed* pack , struct pollfd* pollfds)
{
    memcpy (big_buffer , pack->data_ , pack->size_);
    big_buffer[pack->size_] = '\n';
    big_buffer[pack->size_ + 1] = '\0';

    if (write (fd , big_buffer , pack->size_ + 2) == WRITE_ERR)
    {
        pr_strerr ("Error in writing!");
        return NULL;
    }

    int bytes = 0;
    while (poll (pollfds , 1 , wait_ms) != 0)
    {
        if (pollfds->revents == POLLIN)
        {
            int added = read (fd , big_buffer + bytes , sizeof (big_buffer) - bytes);
            if (added == -1)
            {
                pr_strerr ("read was unproperly ended!");
                return NULL;
            }

            bytes += added;
        }
    }

    big_buffer[bytes] = '\0';
    return big_buffer;
}