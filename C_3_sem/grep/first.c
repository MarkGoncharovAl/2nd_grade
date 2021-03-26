#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include "Common_libs/safe_ops.h"

const pid_t CHILD = 0;
//
//
void close_all(fd_t file, char *map);

char *skip_spaces(char *str);
void do_command(char *command);

void do_ordinary_command(char *command, fd_t input, fd_t output);
void do_grep_command(char **grep, int count);

void print_command_to_pipe(int fd_pipe, const char *main_file);
//
//
int main(int argc, char *argv[])
{
    if (argc != 2)
        ERROR("Not apropriate data for this task!");

    pipe_t data_pipe;
    pipe_s(&data_pipe);
    pid_t main_fork = fork_s();

    if (main_fork != CHILD)
    {
        print_command_to_pipe(data_pipe.out_, argv[1]);
        return 0;
    }

    //!THIS IS CHILD
    while (1)
    {
        int count_symbols = 0;
        read_s(data_pipe.in_, &count_symbols, sizeof(int));
        if (count_symbols == 0)
        {
            return 0;
        }

        char *command = (char *)calloc(count_symbols, sizeof(char));
        read_s(data_pipe.in_, command, count_symbols);

        pid_t new_child = fork_s();
        if (new_child != CHILD)
        {
            do_command(command);
            free(command);
            return 0;
        }
    }
    return 0;
}
//
//
void print_command_to_pipe(int fd_pipe, const char *main_file)
{
    fd_t file = open_s(main_file, O_RDONLY);
    char *file_str = mmap_s(file);

    char *command = NULL;
    char *remaining = file_str;
    while (command = strtok_r(remaining, "\n", &remaining))
    {
        int len = strlen(command);
        write_s(fd_pipe, &len, sizeof(int));
        write_s(fd_pipe, command, len);
    }

    int num = 0;
    write_s(fd_pipe, &num, sizeof(num));
    close_all(file, file_str);
}

void do_command(char *command)
{
    if (!command)
        ERROR("NULL pointer!");

    char *remaining = command;
    char *temp_name = NULL;
    int count = 0;
    while (temp_name = strtok_r(remaining, "|", &remaining))
    {
        count++;
    }

    if (count == 1)
        do_ordinary_command(command, STDIN_FILENO, STDOUT_FILENO);

    //!COUNT NOW  >= 2
    char **data = (char **)calloc(count + 1, sizeof(char *));
    remaining = command;
    for (int i = 0; i < count; ++i)
    {
        data[i] = remaining;
        //printf("%s\n", massive[i]);
        remaining += strlen(remaining) + 1;
    }
    data[count] = NULL;

    do_grep_command(data, count);
}

void do_ordinary_command(char *command, fd_t input, fd_t output)
{
    if (!command)
        ERROR("NULL pointer!");

    if (input != STDIN_FILENO)
        dup2_s(input, STDIN_FILENO);
    if (output != STDOUT_FILENO)
        dup2_s(output, STDOUT_FILENO);

    char *remaining = command;
    int count = 0;
    while (strtok_r(remaining, " ", &remaining))
    {
        count++;
    }

    char **massive = (char **)calloc(count + 1, sizeof(char *));
    remaining = command;
    for (int i = 0; i < count; ++i)
    {
        massive[i] = remaining;
        //printf("%s\n", massive[i]);
        remaining += strlen(remaining) + 1;
    }
    massive[count] = NULL;

    if (execvp(massive[0], massive) == -1)
        ERROR("EXEC IS DIED!");

    free(massive);
}
void do_grep_command(char **grep, int count)
{
    pipe_t *pipes = (pipe_t *)calloc(count - 1, sizeof(pipe_t));
    for (int i = 0; i < count - 1; ++i)
        pipe_s(pipes + i);

    pid_t child = fork_s();

    if (child == CHILD)
    {
        grep[0] = skip_spaces(grep[0]);
        close(pipes[0].in_);
        //printf("%s\n", grep[0]);
        do_ordinary_command(grep[0], STDIN_FILENO, pipes[0].out_);
        return;
    }

    for (int i = 1; i < count - 1; ++i)
    {
        child = fork_s();
        if (child == 0)
        {
            close(pipes[i - 1].out_);
            close(pipes[i].in_);
            grep[i] = skip_spaces(grep[i]);
            //printf("%s\n", grep[i]);
            do_ordinary_command(grep[i], pipes[i - 1].in_, pipes[i].out_);
            return;
        }
        close(pipes[i - 1].in_);
        close(pipes[i - 1].out_);
    }

    child = fork_s();
    if (child == 0)
    {
        close(pipes[count - 2].out_);
        grep[count - 1] = skip_spaces(grep[count - 1]);
        //printf("%s\n", grep[count - 1]);
        do_ordinary_command(grep[count - 1], pipes[count - 2].in_, STDOUT_FILENO);
        return;
    }

    close(pipes[count - 2].in_);
    close(pipes[count - 2].out_);
    free(pipes);
}
//
//
//
//
//
//
//
//
//
//
//
//
//
char *skip_spaces(char *str)
{
    if (!str)
        ERROR("Tried to skip nullptr string!");
    while (isspace(*str))
    {
        str++;
    }
    return str;
}

void close_all(fd_t file, char *map)
{
    struct stat st;
    fstat(file, &st);
    close(file);
    munmap(map, st.st_size);
}
