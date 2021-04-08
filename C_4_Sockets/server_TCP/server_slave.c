#include "../Com_libs/includes.h"
#include "../Com_libs/const.h"
#include "../packet/packet.h"

#define NOT_FOUND_STR "There is no such command!\n"
#define SMALL_BUFF 64
#define wait_ms 1000

static int CheckArgs (int argc , char* argv []);
static int StartSlave ();

static int create_bash ();
static int GetResFd (int fd , struct termios* flags);
static int check_bash (int fd);

static int check_buffer (char* buffer);
static int print_cur_dir ();
static int send_message (char* str);
static int send_message_size (char* str, size_t size);
static int do_ls ();

//NOT dynamic output
static char* write_into_bash (int fd , M_pack_named* pack , struct pollfd* pollfds);

static int SetLogFileID (char* ID);

static int my_socket = 0;
static int port = 0;
static struct sockaddr* name = NULL;
static int pid_parent = 0;
static int big_buffer_size = 0;

#define EXIT_SERVER_SIG SIGUSR1

/* argv:
[1] - socket sending
[2] - sin_port
[3] - sin_addr
[4] - pid-parrent to close it
*/

int main (int argc , char* argv [])
{
    if (CheckArgs (argc , argv) == -1)
        return -1;

    struct in_addr addr = { atoi (argv[3]) };
    struct sockaddr_in sock_addr = { AF_INET, port, addr, 0 };
    name = (struct sockaddr*)(&sock_addr);

    pr_info ("Server slave was initialized");
    send_message("Ready to use!\n");
    StartSlave ();

    pr_info ("Exit programm server_slave");
    close (my_socket);
    return 0;
}

int CheckArgs (int argc , char* argv [])
{
    if (argc != 5)
    {
        pr_err ("Not enough parameters: %d. Must be 4" , argc);
        return -1;
    }

    my_socket = atoi (argv[1]);
    port = atoi (argv[2]);
    pid_parent = atoi (argv[4]);

    if (SetLogFileID ("TCP") == -1)
        return -1;

    pr_info ("Starting logging!");

    return 0;
}

static int StartSlave ()
{
    while (1)
    {
        M_pack_named* pack = M_ReadPack_Named (my_socket, name);
        if (pack == NULL)
            return -1;

        if (strcmp (pack->data_ , "exit") == CMP_EQ)
        {
            M_DestroyPack_Named (pack);
            return 0;
        }

        if (strcmp (pack->data_ , "bash") == CMP_EQ)
        {
            M_DestroyPack_Named (pack);
            return create_bash ();
        }

        if (strcmp (pack->data_ , "CLOSE_SERVER") == CMP_EQ)
        {
            M_DestroyPack_Named (pack);
            pr_info ("Killing server");
            if (kill(pid_parent, EXIT_SERVER_SIG) == -1)
            {
                pr_strerr ("Can't kill pid %d", pid_parent);
                return -1;
            }
            return 0;
        }

        if (check_buffer (pack->data_) == -1)
        {
            M_DestroyPack_Named (pack);
            return -1;
        }

        M_DestroyPack_Named (pack);
    }

    return 0;
}

int SetLogFileID (char* ID)
{
    char buf[100] = {};
    if (sprintf (buf , "/home/mark/VS_prog/2nd_grade/C_4_Sockets/LOG/slave%s.log" , ID) == -1)
    {
        pr_strerr ("Can't create name of log file slaveTCP%s" , ID);
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

    struct termios term;
    term.c_lflag = 0;
    int resfd = GetResFd (fd , &term);
    if (resfd == -1)
        return -1;

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
        exit (1);
    }

    pr_info ("Created bash of server slave");

    int ret = check_bash (fd);
    close (fd);
    close (resfd);
    return ret;
}

int GetResFd (int fd , struct termios* flags)
{
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
    if (tcsetattr (resfd , 0 , flags) == -1)
    {
        pr_strerr ("Can't set tc_attr in %d fd" , resfd);
        return -1;
    }

    return resfd;
}


static int print_first_com (int fd , struct pollfd* poll);
int check_bash (int fd)
{
    pr_info ("Checking bash");
    if (send_message ("Bash was started!\n") == -1)
        return -1;

    struct pollfd pollfds;
    pollfds.fd = fd;
    pollfds.events = POLLIN;

    if (print_first_com (fd , &pollfds) == -1)
        return -1;

    pr_info ("Main checking bash is started!");
    while (1)
    {
        M_pack_named* pack = M_ReadPack_Named (my_socket, name);
        if (pack == NULL)
            return -1;

        pr_info ("Getted message: %s" , pack->data_);
        if (strcmp (pack->data_ , "CLOSE_SERVER") == CMP_EQ)
        {
            M_DestroyPack_Named (pack);
            return -1;
        }

        char* buf = write_into_bash (fd , pack , &pollfds);
        if (buf == NULL)
        {
            M_DestroyPack_Named (pack);
            return -1;
        }

        M_DestroyPack_Named (pack);
        pr_info ("SENDING: %s" , buf);
        if (send_message_size (buf, big_buffer_size) == -1)
            return -1;
    }

    return 0;
}

int print_first_com (int fd , struct pollfd* poll)
{
    M_pack_named* pack = M_CreatePack_Named ("\n" , 1, 0);
    if (pack == NULL)
        return -1;

    char* buf = write_into_bash (fd , pack , poll);
    if (buf == NULL)
        return -1;

    M_DestroyPack_Named (pack);
    return 0;
}

static int do_cd (char* buffer);
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
            if (do_cd (buffer + 2) == -1)
                return -1;
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
int do_cd (char* buffer)
{
    int err = 0;

    if (buffer[0] == ' ')
        err = chdir (buffer + 1);
    else if (buffer[0] == '\0')
        err == chdir ("/");
    else
        err = SOCK_ERR;

    if (err == SOCK_ERR)
    {
        if (send_message ("Can't do this with directories!\n") == -1)
            return -1;
        return 0;
    }

    if (print_cur_dir () == -1)
        return -1;

    return 0;
}


int print_cur_dir ()
{
    char buffer[64] = "DIRECTORY: ";
    if (getcwd (buffer + 11, 51) == NULL)
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

static int cat_strings (int pipes[2] , char* big , char* small , char* dir);
int do_ls ()
{
    pr_info ("Doing ls");

    char Dir_buffer[1024] = "DIRECTORY: ";
    char buffer1[64] = {};

    if (getcwd (buffer1 , 64) == NULL)
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
    char buffer[924] = {};

    if (cat_strings (new_pipe , Dir_buffer , buffer1 , buffer) == -1)
        return -1;

    pr_info ("Ls was done");
    return 0;
}

int cat_strings (int pipes[2] , char* big , char* small , char* dir)
{
    if (write (pipes[1] , "\0" , 1) == WRITE_ERR)
    {
        pr_strerr ("Can't write into pipe %d" , pipes[1]);
        return -1;
    }

    if (read (pipes[0] , dir , 924) == READ_ERR)
    {
        pr_strerr ("Can't read from pipe %d" , pipes[1]);
        return -1;
    }
    else
    {
        strcat (big , small);
        strcat (big , "\n");
        strcat (dir , "\0");
        strcat (big , dir);
        if (send_message (big) == -1)
            return -1;
    }

    close (pipes[0]);
    close (pipes[1]);
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

int send_message_size (char* str, size_t size)
{
    pr_info ("Sending message");
    M_pack_named* packet = M_CreatePack_Named_Mem (str , size , 0);
    if (packet == NULL)
        return -1;

    if (M_WritePack_Named (my_socket , name , packet) == -1)
        return -1;

    free (packet);
    pr_info ("Message was sent");
    return 0;
}


static char big_buffer[64 * BUFSZ] = {};
char* write_into_bash (int fd , M_pack_named* pack , struct pollfd* pollfds)
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

    while (big_buffer[bytes - 1] != '$')
        bytes--;
    big_buffer[bytes] = ' ';
    big_buffer[bytes + 1] = '\0';
    big_buffer_size = bytes + 2;

    return big_buffer;
}