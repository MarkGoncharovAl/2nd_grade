#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

typedef int fd_t;
#define O_ERROR -1

int main(int argc, char *argv[])
{
    //open(O_NONBLOCK) - запись произойдёт мгновенно
    //fcntl - можно перевести в nonblock режим
    //F_GETFL + F_SETFL
    //man 2 poll - ожидание на дескрипторах

    if (argc < 2)
    {
        printf("Not enough parameters for this task!\n");
        return 0;
    }

    fd_t *fds = (fd_t *)calloc(argc - 1, sizeof(fd_t));
    struct pollfd *fds_poll = (struct pollfd *)calloc(argc - 1, sizeof(struct pollfd));

    for (int i = 1; i < argc; ++i)
    {
        fds[i - 1] = mkfifo(argv[i], O_CREAT | 0666);
        if (fds[i - 1] == O_ERROR && errno != EEXIST)
        {
            perror("mkfifo mistake!");
            return -1;
        }

        fds[i - 1] = open(argv[i], O_NONBLOCK);
        if (fds[i - 1] == O_ERROR)
        {
            perror("open mistake!");
            return -1;
        }

        fds_poll[i - 1].fd = fds[i - 1];
        fds_poll[i - 1].events = POLLIN;
    }

    while (poll(fds_poll, argc - 1, -1) != O_ERROR)
    {
        //now ret > 0
        for (int i = 0; i < argc - 1; ++i)
        {
            if (fds_poll[i].revents & POLLIN)
            {
                fds_poll[i].revents = 0;

                printf("Printing changed file: %s!\n", argv[i + 1]);
                pid_t pd = fork();
                if (pd == 0)
                {
                    int out = execlp("cat", "cat", argv[i + 1], NULL);
                }
            }
        }
    }

    free(fds);
    free(fds_poll);
}