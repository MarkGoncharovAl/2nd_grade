#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>

typedef int fd_t;
#define MAX_EVENTS 10

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        perror("Not enough amount of parameters!");
        return 0;
    }

    fd_t in = inotify_init();
    if (in == -1)
    {
        perror("Init is bad!\n");
        return -1;
    }

    uint32_t mask = IN_CREATE | IN_DELETE | IN_MOVE_SELF;
    int watch = inotify_add_watch(in, argv[1], mask);

    if (watch == -1)
    {
        perror("Watch is bad!\n");
        return -1;
    }

    struct inotify_event buf[MAX_EVENTS];
    while (1)
    {
        int check_read = read(in, (void *)buf, sizeof(struct inotify_event) * MAX_EVENTS) / sizeof(struct inotify_event);
        for (int i = 0; i < check_read; ++i)
        {
            struct inotify_event *event = buf + i;
            printf("Path: %s ", event->name);
            if (event->mask & IN_CREATE)
                printf("Event is creating!\n");
            if (event->mask & IN_DELETE)
                printf("Event is deleting!\n");
            if (event->mask & IN_MOVE_SELF)
                printf("Event is moving now!\n");
        }
    }

    close(in);
    return 0;
}

// !For next homework
//2 fork()
//man 2 setsid, setgit
// parent = init
// man 7 daemon