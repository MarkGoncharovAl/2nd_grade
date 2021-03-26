#include "safe_ops.h"

void pipe_safe(pipe_t *data, int LINE, const char *FILE)
{
    int data_pipe[2] = {};
    int out_pipe = pipe(data_pipe);
    if (out_pipe == -1)
        errror("PIPE!", LINE, FILE);

    data->in_ = data_pipe[0];
    data->out_ = data_pipe[1];
}

pid_t fork_safe(int LINE, const char *FILE)
{
    pid_t out = fork();
    if (out == -1)
        errror("FORK!", LINE, FILE);

    return out;
}

fd_t open_safe(const char *file, int O_FLAG, int LINE, const char *FILE)
{
    fd_t output = open(file, O_FLAG);

    struct stat st;
    fstat(output, &st);

    if (output < 0)
        errror("Can't open this file!", LINE, FILE);

    return output;
}

void *mmap_safe(fd_t file, int LINE, const char *FILE)
{
    struct stat st;
    fstat(file, &st);

    void *out = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, file, 0);

    if (out == (void *)(-1))
        errror("Mmaping", LINE, FILE);

    return out;
}

void dup2_safe(fd_t old_fd, fd_t new_fd, int LINE, const char *FILE)
{
    int out_check = dup2(old_fd, new_fd);
    if (out_check == -1)
        errror("DUP2", LINE, FILE);
}
void write_safe(fd_t fd, void *buf, size_t size, int LINE, const char *FILE)
{
    ssize_t out_check = write(fd, buf, size);
    if (out_check == -1)
        errror("WRITE", LINE, FILE);
}
void read_safe(fd_t fd, void *buf, size_t size, int LINE, const char *FILE)
{
    ssize_t out_check = read(fd, buf, size);
    if (out_check == -1)
        errror("READ", LINE, FILE);
}

void errror(char info[], size_t LINE, const char *FILE)
{
    printf("Mistake was found in %Ilu in file: %s", LINE, FILE);
    printf("\n\nProblem: %s\n", info);
    perror("Problem in perror");
    printf("\n");
    abort();
}

void warrning(char info[], size_t LINE, const char *FILE)
{
    printf("Warning was found in %Ilu in file: %s", LINE, FILE);
    printf("\n\nProblem: %s\n", info);
    perror("Problem in perror");
    printf("\n");
}

DIR *opendir_safe(const char *filename, int LINE, const char *FILE)
{
    DIR *file = opendir(filename);
    if (file == NULL)
        errror("Mistake in opening directory!", LINE, FILE);

    return file;
}