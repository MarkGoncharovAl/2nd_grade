#pragma once

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>

typedef char bool_t;
#define true 1
#define false 0

typedef int fd_t;
typedef struct
{
    fd_t in_;
    fd_t out_;
} pipe_t;

void errror(char info[], size_t LINE, const char *FILE);

#define ERROR(a) errror(a, __LINE__, __FILE__)

pid_t fork_safe(int LINE);
void pipe_safe(pipe_t *data, int LINE);
void dup2_safe(fd_t old_fd, fd_t new_fd, int LINE);

void write_safe(fd_t fd, void *buf, size_t size, int LINE);
void read_safe(fd_t fd, void *buf, size_t size, int LINE);

fd_t open_safe(const char *file, int O_FLAG, int LINE);

char *mmap_safe(fd_t file, int LINE);

#define pipe_s(a) pipe_safe(a, __LINE__)
#define fork_s() fork_safe(__LINE__)
#define open_s(a, b) open_safe(a, b, __LINE__)
#define mmap_s(a) mmap_safe(a, __LINE__)
#define write_s(a, b, c) write_safe(a, b, c, __LINE__)
#define read_s(a, b, c) read_safe(a, b, c, __LINE__)
#define dup2_s(a, b) dup2_safe(a, b, __LINE__)

void pipe_safe(pipe_t *data, int LINE)
{
    int data_pipe[2] = {};
    int out_pipe = pipe(data_pipe);
    if (out_pipe == -1)
        ERROR("PIPE!");

    data->in_ = data_pipe[0];
    data->out_ = data_pipe[1];
}

pid_t fork_safe(int LINE)
{
    pid_t out = fork();
    if (out == -1)
        ERROR("FORK!");

    return out;
}

fd_t open_safe(const char *file, int O_FLAG, int LINE)
{
    fd_t output = open(file, O_RDONLY);

    struct stat st;
    fstat(output, &st);

    if (output < 0 || !S_ISREG(st.st_mode))
        ERROR("Opening not reg file");

    return output;
}

char *mmap_safe(fd_t file, int LINE)
{
    struct stat st;
    fstat(file, &st);

    void *out = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, file, 0);

    if (out == (void *)(-1))
        ERROR("Mmaping");

    return (char *)out;
}

void dup2_safe(fd_t old_fd, fd_t new_fd, int LINE)
{
    int out_check = dup2(old_fd, new_fd);
    if (out_check == -1)
        ERROR("DUP2");
}
void write_safe(fd_t fd, void *buf, size_t size, int LINE)
{
    ssize_t out_check = write(fd, buf, size);
    if (out_check == -1)
        ERROR("WRITE");
}
void read_safe(fd_t fd, void *buf, size_t size, int LINE)
{
    ssize_t out_check = read(fd, buf, size);
    if (out_check == -1)
        ERROR("READ");
}

void errror(char info[], size_t LINE, const char *FILE)
{
    printf("Mistake was found in %Ilu in file: %s", LINE, FILE);
    printf("\n\nProblem: %s\n", info);
    perror("Problem in perror");
    printf("\n");
    assert(0);
}
