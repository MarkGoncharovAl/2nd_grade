#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

int main(int argc, char **argv)
{
    char **my_envp = (char **)calloc(argc - 1, sizeof(char *));
    for (int i = 0; i < argc - 2; ++i)
    {
        my_envp[i] = argv[i + 2];
    }
    my_envp[argc - 2] = NULL;

    pid_t pd = fork();
    if (pd == 0)
        execve(argv[1], argv + 1, my_envp);

    waitpid(pd, NULL, 0);
    free(my_envp);
    return 0;
}