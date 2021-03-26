#include "ResCop.h"

///////////////////////
//!Tables of pids
typedef struct
{
    pid_t pd_;
    char *file_;
} Node;
Node TABLE[100];
int CUR_NODE = 0;
fd_t IN = 0;
char flag_end = 1;
///////////////////////

char *AddAsFolder(const char *folder)
{
    int length = strlen(folder);

    char *ret = (char *)calloc(length + 2, sizeof(char));
    memcpy(ret, folder, length);

    ret[length] = '/';
    ret[length + 1] = '\0';
    return ret;
}

bool_t CheckFolder(const char *from, const char *to)
{
    DIR *check = opendir(to);
    if (check == NULL)
    {
        if (fork() == 0)
            execlp("cp", "cp", "-r", from, to, NULL);
        return False;
    }
    else
    {
        pid_t pd = fork();
        if (pd == 0)
            execlp("rm", "rm", "-r", to, NULL);
        waitpid(pd, NULL, 0);
        if (fork() == 0)
            execlp("cp", "cp", "-r", from, to, NULL);
    }

    return True;
}

int CreateWatch(fd_t in, const char *folder)
{
    uint32_t mask = IN_CREATE | IN_ATTRIB | IN_DELETE | IN_DELETE_SELF | IN_MODIFY | IN_MOVE_SELF | IN_MOVED_FROM | IN_MOVED_TO;
    int watch = inotify_add_watch(in, folder, mask);

    if (watch == -1)
        ERROR("Watch is bad!\n");

    return watch;
}

char *GetFullName(const char *folder, const char *file)
{
    char *final_file = (char *)calloc(strlen(folder) + strlen(file) + 1, sizeof(char));
    final_file[0] = '\0';

    if (strcat(final_file, folder) == NULL)
        ERROR("First strcat!");

    if (strcat(final_file, file) == NULL)
        ERROR("Second strcat!");

    return final_file;
}

void CopyFiles(const char *folder1, const char *file1, const char *folder2, const char *file2)
{
    char *file_1 = GetFullName(folder1, file1);
    char *file_2 = GetFullName(folder2, file2);
    if (file_1 == NULL || file_2 == NULL)
        ERROR("Can't start function CopyFiles!");

    pid_t pd = fork_s();
    if (pd == 0)
    {
        int out = execlp("cp", "cp", file_1, file_2, NULL);
        if (out == -1)
            ERROR("Execp wasn't done!");
    }

    waitpid(pd, NULL, 0);
    free(file_1);
    free(file_2);
}

void CopyFolders(const char *folder1, const char *file1, const char *folder2, const char *file2, fd_t log)
{
    char *file_1 = GetFullName(folder1, file1);
    char *file_2 = GetFullName(folder2, file2);

    if (file_1 == NULL || file_2 == NULL)
        ERROR("Can't start function CopyFolders!");

    pid_t pd = fork_s();
    if (pd == 0)
    {
        int out = execlp("cp", "cp", "-r", file_1, file_2, NULL);
        if (out == -1)
            ERROR("Execlp cp -r wasn't done!");
    }

    waitpid(pd, NULL, 0);
    AddWatch(file_1, file_2, log);
    free(file_1);
    free(file_2);
}

void DeleteFile(const char *folder, char *file)
{
    char *full_name = GetFullName(folder, file);

    pid_t pd = fork_s();
    if (pd == 0)
    {
        int out = execlp("rm", "rm", full_name, NULL);
        if (out == -1)
            ERROR("execlp rm!");
    }

    waitpid(pd, NULL, 0);
    free(full_name);
}

void DeleteFolder(const char *folder, char *file)
{
    char *full_name = GetFullName(folder, file);

    for (int i = 0; i < CUR_NODE; ++i)
    {
        if (!strcmp(full_name, TABLE[i].file_))
        {
            //write (log, "Deleting folder: %s\n", full_name);
            kill(TABLE[i].pd_, SIGUSR1);
            CUR_NODE--;

            if (i != CUR_NODE)
            {
                //free(TABLE[i].file_);
                TABLE[i].pd_ = TABLE[CUR_NODE].pd_;
                TABLE[i].file_ = TABLE[CUR_NODE].file_;
            }
        }
    }

    pid_t pd = fork_s();
    if (pd == 0)
    {
        int out = execlp("rm", "rm", "-r", full_name, NULL);
        if (out == -1)
            ERROR("execlp rm -r!");
    }

    waitpid(pd, NULL, 0);
    free(full_name);
}

void ProcessFolder(uint32_t mask, char *name, char *from, char *to, fd_t log)
{
    if (mask & IN_CREATE)
    {
        char buf[] = "Creating folder!\n";
        write(log, (void *)buf, strlen(buf));
        CopyFolders(from, name, to, name, log);
    }
    if (mask & IN_DELETE)
    {
        char buf[] = "Deleting folder!\n";
        write(log, (void *)buf, strlen(buf));
        DeleteFolder(to, name);
    }
    if (mask & IN_ATTRIB)
    {
        char buf[] = "Deleting folder!\n";
        write(log, (void *)buf, strlen(buf));
    }
    if (mask & IN_DELETE_SELF)
    {
        char buf[] = "Deleting_self folder!\n";
        write(log, (void *)buf, strlen(buf));
        raise(SIGUSR1);
    }
    if (mask & IN_MODIFY)
    {
        char buf[] = "Modifying folder!\n";
        write(log, (void *)buf, strlen(buf));
        CopyFolders(from, name, to, name, log);
    }
    if (mask & IN_MOVE_SELF)
    {
        char buf[] = "Moving self folder!\n";
        write(log, (void *)buf, strlen(buf));
    }

    if (mask & IN_MOVED_FROM)
    {
        char buf[] = "Moving_from folder!\n";
        write(log, (void *)buf, strlen(buf));
        DeleteFolder(to, name);
    }
    if (mask & IN_MOVED_TO)
    {
        char buf[] = "Moving into folder!\n";
        write(log, (void *)buf, strlen(buf));
        CopyFolders(from, name, to, name, log);
    }
}
void ProcessFile(uint32_t mask, char *name, char *from, char *to, fd_t log)
{
    if (mask & IN_CREATE)
    {
        char buf[] = "Creating file!\n";
        write(log, (void *)buf, strlen(buf));
        CopyFiles(from, name, to, name);
    }
    if (mask & IN_DELETE)
    {
        char buf[] = "Deleting file!\n";
        write(log, (void *)buf, strlen(buf));
        DeleteFile(to, name);
    }
    if (mask & IN_ATTRIB)
    {
        char buf[] = "Attributing file!\n";
        write(log, (void *)buf, strlen(buf));
    }
    if (mask & IN_DELETE_SELF)
    {
        char buf[] = "Deleting_self file!\n";
        write(log, (void *)buf, strlen(buf));
    }
    if (mask & IN_MODIFY)
    {
        char buf[] = "Modifying file!\n";
        write(log, (void *)buf, strlen(buf));
        CopyFiles(from, name, to, name);
    }
    if (mask & IN_MOVE_SELF)
    {
        char buf[] = "Moving_Self file!\n";
        write(log, (void *)buf, strlen(buf));
    }

    if (mask & IN_MOVED_FROM)
    {
        char buf[] = "Moving from file!\n";
        write(log, (void *)buf, strlen(buf));
        DeleteFile(to, name);
    }
    if (mask & IN_MOVED_TO)
    {
        char buf[] = "Moving to file!\n";
        write(log, (void *)buf, strlen(buf));
        CopyFiles(from, name, to, name);
    }
}

int AddWatch(char *from, char *to, fd_t log)
{
    pid_t pd = fork();
    if (pd != 0)
    {
        TABLE[CUR_NODE].file_ = from;
        TABLE[CUR_NODE].pd_ = pd;
        CUR_NODE++;
        return pd;
    }

    //!Now kid!
    CUR_NODE = 0;
    signal(SIGUSR1, SIG_USR2);

    char *FOLDER_FROM = AddAsFolder(from);
    char *FOLDER_TO = AddAsFolder(to);

    CheckFolder(FOLDER_FROM, FOLDER_TO);

    IN = inotify_init();
    if (IN == -1)
        ERROR("Init is bad!\n");

    //write ("%s\n", FOLDER_FROM);
    CreateWatch(IN, FOLDER_FROM);

    struct inotify_event events[MAX_SIZE];
    while (flag_end)
    {
        int check_read = read(IN, (void *)events, sizeof(struct inotify_event) * MAX_SIZE) / sizeof(struct inotify_event);

        if (check_read == -1)
        {
            //WARNING("Reading was unsecessfull!");
            break;
        }

        for (int i = 0; i < check_read - 1; ++i)
        {
            struct inotify_event *event = events + i;

            if (strlen(event->name) > 3)
            {
                char *buf = CreateOut(event->name);
                write(log, buf, strlen(buf));
                free(buf);
            }

            if (event->mask & IN_ISDIR)
                ProcessFolder(event->mask, event->name, FOLDER_FROM, FOLDER_TO, log);
            else
                ProcessFile(event->mask, event->name, FOLDER_FROM, FOLDER_TO, log);
        }
    }

    close(log);
    free(to);
    free(from);
    raise(SIGKILL);
    return pd;
}

void SIG_USR2(int fd)
{
    if (fd == 10)
    {
        //printf("Killing!\n");
        for (int i = 0; i < CUR_NODE; ++i)
            kill(TABLE[i].pd_, SIGUSR1);

        close(IN);
        flag_end = 0;
    }
    //else
    //printf("SIGNAL %d\n", fd);
}

char *CreateOut(char *file)
{
    if (file == NULL)
    {
        WARNING("Not apprepriate pointer to function!");
        return NULL;
    }

    int length = 8 + strlen(file);
    char *buf = (char *)calloc(length, sizeof(char));
    strcat(buf, "Path: ");
    strcat(buf, file);
    buf[length - 2] = '\n';
    buf[length - 1] = '\0';
    return buf;
}