#include "Libraries/safe_ops.h"

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3)
        ERROR("Not enough parameters for changing current directories!");

    int out = open_s("/tmp/daemon_way", O_WRONLY);

    if (argc == 2)
    {
        if (strcmp(argv[1], "STOP"))
        {
            printf("Can't find command <%s>\n", argv[1]);
            return 0;
        }

        //!Now command is stop
        printf("Stopping work of daemon!\n");
        int num = -1;
        write_s(out, (void *)(&num), sizeof(int));
        return 0;
    }

    //!Now argc == 3
    printf("Changing directories for daemon!\n");
    int num = 0;
    write_s(out, (void *)(&num), sizeof(int));

    num = strlen(argv[1]);
    write_s(out, (void *)(&num), sizeof(int));
    write_s(out, (void *)(argv[1]), num);

    num = strlen(argv[2]);
    write_s(out, (void *)(&num), sizeof(int));
    write_s(out, (void *)(argv[2]), num);
    close(out);
}