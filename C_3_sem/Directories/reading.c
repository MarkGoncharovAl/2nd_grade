#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

int main(int argc, char *argv[])
{
    /*
    DIR *opendir(const char *name);
    DIR *fdopendir(int fd);
    readdir 3
    DT_
    rewinddir
    seekdir
    scandir - DON'T
    telldir
    closedir
    */

    for (int i = 1; i < argc; ++i)
    {
        DIR *dir = opendir(argv[i]);
        if (dir == NULL)
        {
            printf("File %s can't be opened!\n", argv[i]);
            continue;
        }

        struct dirent *out;
        while ((out = readdir(dir)) != NULL)
        {
            printf("%s\n", out->d_name);
        }

        closedir(dir);
    }
}