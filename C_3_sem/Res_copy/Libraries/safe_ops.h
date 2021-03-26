#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>

//  !DON'T FORGET
//  !1) if you use shared memory you've got to have
//  !   file SHARED_MEMORY

typedef char bool_t;
#define True 1
#define False 0

typedef int fd_t;

typedef int sem_t;
#define MY_IPC_ERROR -1

typedef struct
{
    fd_t in_;
    fd_t out_;
} pipe_t;

//  !Checking errors
//////////////////////////////////////////////
void errror(char info[], size_t LINE, const char *FILE);
void warrning(char info[], size_t LINE, const char *FILE);
#define ERROR(a) errror(a, __LINE__, __FILE__)
#define WARNING(a) warrning(a, __LINE__, __FILE__)
//////////////////////////////////////////////

pid_t fork_safe(int LINE, const char *FILE);
void pipe_safe(pipe_t *data, int LINE, const char *FILE);
void dup2_safe(fd_t old_fd, fd_t new_fd, int LINE, const char *FILE);

void write_safe(fd_t fd, void *buf, size_t size, int LINE, const char *FILE);
void read_safe(fd_t fd, void *buf, size_t size, int LINE, const char *FILE);

fd_t open_safe(const char *file, int O_FLAG, int LINE, const char *FILE);

void *mmap_safe(fd_t file, int LINE, const char *FILE);

DIR *opendir_safe(const char *filename, int LINE, const char *FILE);

#define pipe_s(a) pipe_safe(a, __LINE__, __FILE__)
#define fork_s() fork_safe(__LINE__, __FILE__)
#define open_s(a, b) open_safe(a, b, __LINE__, __FILE__)
#define mmap_s(a) mmap_safe(a, __LINE__, __FILE__)
#define write_s(a, b, c) write_safe(a, b, c, __LINE__, __FILE__)
#define read_s(a, b, c) read_safe(a, b, c, __LINE__, __FILE__)
#define dup2_s(a, b) dup2_safe(a, b, __LINE__, __FILE__)
#define opendir_s(a) opendir_safe(a, __LINE__, __FILE__)