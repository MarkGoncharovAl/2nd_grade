#include "fast_use.h"
int FastOpen(const char * file)
{
    int err = open(file, O_RDWR | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);
    if (err == -1)
        pr_strerr("Can't open file %s!", file);
    
    return err;
}