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

int main(int argc, char *argv[], char *envp[])
{
    //There will be a programm
    printf("Checking new envp:\n");
    while (*envp != NULL)
    {
        printf("%s\n", *envp);
        envp++;
    }
    return 0;
}