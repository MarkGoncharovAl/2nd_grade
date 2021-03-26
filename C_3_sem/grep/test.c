#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include "Common_libs/safe_ops.h"

int main(int argc, char *argv[])
{
    if (execlp("history", "history", NULL) == -1)
        printf("1111");
}