#include "../Com_libs/includes.h"
#include "../Com_libs/const.h"
#include "../packet/packet.h"

#define NOT_FOUND_STR "There is no such command!\n"
#define SMALL_BUFF 64
#define wait_ms 1000

static int CheckArgs (int argc , char* argv []);
static int StartSlave ();

static int CreateBash ();
static int GetResFd (int fd , struct termios* flags);
static int CheckBash (int fd);

static int CheckBuffer (char* buffer);
static int PrintCurDir ();
static int SendMessage (char* str);
static int SendMessageSize (char* str , size_t size);
static int MakeLs ();

//dynamic output
static char* WriteIntoBash (int fd , M_pack_unnamed* pack , struct pollfd* pollfds);

static int SetLogFileID (char* ID);

static int pipe_rd = 0;
static int my_socket = 0;
static int port = 0;
static struct sockaddr* name = NULL;
static char* ID = NULL;
static int big_buffer_size = 0;

/* argv:
[1] - pipe_reading
[2] - socket sending
[3] - sin_port
[4] - sin_addr
[5] - ID
*/

int main (int argc , char* argv [])
{
    if (CheckArgs (argc , argv) == -1)
        return -1;

    struct in_addr addr = { atoi (argv[4]) };
    struct sockaddr_in sock_addr = { AF_INET, port, addr, 0 };
    name = (struct sockaddr*)(&sock_addr);

    if (SendMessage (argv[5]) == -1) //ID CLIENT
        return -1;

    pr_info ("Server slave was initialized");
    StartSlave ();

    pr_info ("Exit programm server_slave");
    close (pipe_rd);
    return 0;
}

int CheckArgs (int argc , char* argv [])
{
    if (SetLogFileID (argv[5]) == -1)
        return -1;

    pr_info ("Starting logging!");

    if (argc != 6)
    {
        pr_err ("Not enough parameters: %d. Must be 6" , argc);
        return -1;
    }

    pipe_rd = atoi (argv[1]);
    my_socket = atoi (argv[2]);
    port = atoi (argv[3]);
    return 0;
}

static int StartSlave ()
{
    M_pack_unnamed* pack = NULL;
    int ret = 0;

    while (1)
    {
        pack = M_ReadPack_Unnamed (pipe_rd);
        if (pack == NULL)
            return 1;

        if (strcmp (pack->data_ , "CLOSE_SERVER") == CMP_EQ)
            EXIT1 (0);

        if (strcmp (pack->data_ , "bash") == CMP_EQ)
            EXIT1(CreateBash ());

        if (CheckBuffer (pack->data_) == -1)
            EXIT(-1);

        M_DestroyPack_Unnamed (pack);
    }


exit1:
    M_DestroyPack_Unnamed (pack);
    return 0;
}

int SetLogFileID (char* ID)
{
    char buf[100] = {};
    if (sprintf (buf , "/var/log/slave%s.log" , ID) == -1)
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

int CreateBash ()
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

    int ret = CheckBash (fd);
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


static int PrintFirstCommand (int fd , struct pollfd* poll);
int CheckBash (int fd)
{
    pr_info ("Checking bash");
    int ret = 0;
    if (SendMessage ("Bash was started!\n") == -1)
        return -1;

    struct pollfd pollfds;
    pollfds.fd = fd;
    pollfds.events = POLLIN;

    if (PrintFirstCommand (fd , &pollfds) == -1)
        return -1;

    pr_info ("Main checking bash is started!");
    M_pack_unnamed* pack = NULL;
    while (1)
    {
        pack = M_ReadPack_Unnamed (pipe_rd);

        if (pack == NULL)
            return -1;

        pr_info ("Getted message: %s" , pack->data_);
        if (strcmp (pack->data_ , "CLOSE_SERVER") == CMP_EQ)
            EXIT1 (-1);

        char* buf = WriteIntoBash (fd , pack , &pollfds);
        if (buf == NULL)
            EXIT1 (-1);

        M_DestroyPack_Unnamed (pack);
        pr_info ("SENDING: %s" , buf);
        if (SendMessageSize (buf , big_buffer_size) == -1)
            return -1;
    }

exit1:
    M_DestroyPack_Unnamed (pack);
    return ret;
}

int PrintFirstCommand (int fd , struct pollfd* poll)
{
    M_pack_unnamed* pack = M_CreatePack_Unnamed ("\n" , 1);
    if (pack == NULL)
        return -1;

    char* buf = WriteIntoBash (fd , pack , poll);
    if (buf == NULL)
        return -1;

    M_DestroyPack_Unnamed (pack);
    return 0;
}

static int MakeCD (char* buffer);
int CheckBuffer (char* buffer)
{
    pr_info ("Checking buffer: %s" , buffer);

    int ret = 0; //1 - not found, -1 - error
    switch (buffer[0])
    {
    case 'c':
        if (buffer[1] == 'd')
            ret = MakeCD (buffer + 2);
        else
            ret = 1;
        break;

    case 'l':
        if (strcmp (buffer , "ls") == CMP_EQ)
            ret = MakeLs ();
        else
            ret = 1;
        break;

    case 'p':
        if (strcmp (buffer , "print") == CMP_EQ)
            ret = SendMessage (DUMMY_STR);
        else
            ret = 1;
        break;
    default:
        ret = 1;
    }

    if (ret == 1)
        ret = SendMessage (NOT_FOUND_STR);

    return ret;
}
int MakeCD (char* buffer)
{
    int err = 0;

    if (buffer[0] == ' ')
        err = chdir (buffer + 1);
    else if (buffer[0] == '\0')
        err == chdir ("/");
    else
        err = -1;

    if (err == -1)
        err = SendMessage ("Can't do this with directories!\n");

    if (err != -1)
        err = PrintCurDir ();

    return err;
}


int PrintCurDir ()
{
    char buffer[64] = "DIRECTORY: ";
    int err = 0;

    if (getcwd (buffer + 11 , 51) == NULL)
        err = SendMessage ("nothing...\n\0");
    else
    {
        strcat (buffer , "\n\0");
        err = SendMessage (buffer);
    }

    return err;
}

static int cat_strings (int pipes[2] , char* big , char* small , char* dir);
int MakeLs ()
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
        if (SendMessage (big) == -1)
            return -1;
    }

    close (pipes[0]);
    close (pipes[1]);
    return 0;
}

int SendMessage (char* str)
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

int SendMessageSize (char* str , size_t size)
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


char big_buffer[64 * BUFSZ] = {};
char* WriteIntoBash (int fd , M_pack_unnamed* pack , struct pollfd* pollfds)
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