#include "Libraries/ResCop.h"

char main_end = 1;
#define KILL_AIM(a) kill(a, SIGUSR1)
#define FIRST_AIM -1

char *ReadNameFile(fd_t fd);

int main()
{
    if (fork() != 0)
        return 0;

    ////////////////////////////////////
    //!Work with desckriptors!
    fd_t fd = open("log.txt", O_WRONLY | O_TRUNC);
    if (fd == -1)
        ERROR("Can't open log file log.txt properly!");

    int check_mkfifo = mkfifo("/tmp/daemon_way", 0666);
    if (check_mkfifo == -1 && errno != EEXIST)
        ERROR("Can't properly create fifo!");

    fd_t fd_fifo = open("/tmp/daemon_way", O_RDONLY);
    if (fd_fifo == -1)
        ERROR("Can't open fifo /tmp/daemon_way properly!");
    ////////////////////////////////////

    char *FileIn = NULL, *FileOut = NULL;
    int last_aim = FIRST_AIM;

    char buf1[] = "Reserve copy has started!\n";
    write_s(fd, (void *)buf1, strlen(buf1));

    while (main_end)
    {
        int num = -2;
        while (read(fd_fifo, (void *)(&num), sizeof(int)) != sizeof(int))
        {
        }

        //!Exit case
        if (num == -1)
            main_end = 0;

        //!Continue case with new data
        else if (num == 0)
        {
            if (last_aim != FIRST_AIM)
            {
                kill(last_aim, SIGUSR1);
                free(FileIn);
                free(FileOut);
            }

            FileIn = ReadNameFile(fd_fifo);
            FileOut = ReadNameFile(fd_fifo);
            last_aim = AddWatch(FileIn, FileOut, fd);
            printf("Folders changed!\nInput: %s\nOutput: %s\n", FileIn, FileOut);
        }

        //!If num != 0 and num != -1
        else
        {
            kill(last_aim, SIGUSR1);
            ERROR("Not appropriate data in fifo /tmp/daemon_pid!");
        }
    }

    if (last_aim != FIRST_AIM)
    {
        free(FileIn);
        free(FileOut);
        KILL_AIM(last_aim);
    }

    char buf2[] = "Reserve copy has ended!\n";
    write_s(fd, (void *)buf2, strlen(buf2));
    close(fd);
    close(fd_fifo);

    printf("Daemon has died!\n");
}

char *ReadNameFile(fd_t fd)
{
    int num = 0;
    read_s(fd, (void *)(&num), sizeof(int));

    char *out = (char *)calloc(num + 1, sizeof(char));
    read(fd, (void *)(out), num);

    return out;
}