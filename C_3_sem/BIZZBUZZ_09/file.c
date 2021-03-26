#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>

void errror(char info[], size_t LINE, const char *FILE);
#define ERROR(a) errror(a, __LINE__, __FILE__)
typedef int fd_type;

typedef char bool;
const bool true = 1;
const bool false = 0;

typedef enum
{
    END = 0,
    LETTERS,
    NOTHING,
    THREE,
    FIVE,
    FIFTEEN
} BIZZ_BUZZ;

//from to
typedef struct
{
    fd_type input_;
    fd_type output_;
} fd;

int ok(int fd_read);

fd_type open_file(const char *file, int O_FLAG);
fd read_1_file(const char *in);
fd read_2_files(const char *in, const char *out);

void copy_files(fd files);
void safe_write(int file, const char *ptr, __off_t bytes);

void bizz_buzz(fd files);
void write_bizz(BIZZ_BUZZ num, int output_file);
const char *write_word(const char *str, int output_file);

BIZZ_BUZZ read_num(const char **str);
BIZZ_BUZZ compare_dels(int del3, int del5);

const char *miss_symbol(const char *str, int output_file);

void close_files(fd files);

int main(int argc, char **argv)
{
    fd files;
    if (argc == 2)
        files = read_1_file(argv[1]);
    else if (argc == 3)
        files = read_2_files(argv[1], argv[2]);
    else
    {
        //argc != 2 != 3
        ERROR("The input format isn't right!");
    }

    //copy_files (files);
    bizz_buzz(files);
    close_files(files);
    return 0;
}

fd_type open_file(const char *file, int O_FLAG)
{
    fd_type output = open(file, O_FLAG);

    struct stat st;
    fstat(output, &st);
    if (output < 0 || !S_ISREG(st.st_mode))
        perror("Opening file");

    return output;
}

fd read_1_file(const char *in)
{
    fd result;
    result.input_ = open_file(in, O_RDONLY);

    char name_out[] = "LOOK_AT_ME";
    result.output_ = open(name_out, O_CREAT | O_WRONLY | O_TRUNC, 0777);

    if (result.output_ < 0)
        ERROR("File wasn't created properly!");

    return result;
}
fd read_2_files(const char *in, const char *out)
{
    fd result;
    result.input_ = open_file(in, O_RDONLY);

    result.output_ = open(out, O_CREAT | O_WRONLY | O_TRUNC, 0777);
    if (result.output_ < 0)
        ERROR("File wasn't opened or created properly!");

    return result;
}

void copy_files(fd files)
{
    struct stat stat_input;
    fstat(files.input_, &stat_input);

    void *ptr_input = mmap(NULL, stat_input.st_size, PROT_READ, MAP_PRIVATE, files.input_, 0);
    safe_write(files.output_, ptr_input, stat_input.st_size);
    munmap(ptr_input, stat_input.st_size);
}

//BIIZ_BUZZ
//------------------------------------------------------
void bizz_buzz(fd files)
{
    struct stat stat_input;
    fstat(files.input_, &stat_input);

    char *ptr_input = (char *)mmap(NULL, stat_input.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, files.input_, 0);

    const char *str_strtok = miss_symbol(ptr_input, files.output_);

    while (str_strtok != NULL && *str_strtok != '\0' && *str_strtok != EOF)
    {
        const char *start_point = str_strtok;
        BIZZ_BUZZ num = read_num(&str_strtok);
        switch (num)
        {
        case NOTHING:
        case LETTERS:
            str_strtok = start_point;
            str_strtok = write_word(str_strtok, files.output_);
            break;
        case END:
            ERROR("Something went really strange...");
            break;
        default:
            write_bizz(num, files.output_);
            break;
        }

        str_strtok = miss_symbol(str_strtok, files.output_);
    }

    munmap(ptr_input, stat_input.st_size);
}
const char *miss_symbol(const char *str, int output_file)
{
    if (str == NULL)
        return NULL;

    char cur_sym = *str;
    while (str != NULL && cur_sym != '\0' && cur_sym != EOF && (cur_sym == ' ' || cur_sym == '\n' || cur_sym == '\t'))
    {
        safe_write(output_file, str, 1);
        str++;
        if (str != NULL)
            cur_sym = *str;
    }
    return str;
}
void write_bizz(BIZZ_BUZZ num, int output)
{
    static const char out1[] = "bizzbuzz";
    static const char out2[] = "bizz";
    static const char out3[] = "buzz";

    switch (num)
    {
    case FIFTEEN:
        safe_write(output, out1, sizeof(out1));
        break;
    case THREE:
        safe_write(output, out2, sizeof(out2));
        break;
    case FIVE:
        safe_write(output, out3, sizeof(out3));
        break;
    default:
        ERROR("Something went wrong...");
        break;
    }
}
const char *write_word(const char *str, int output_file)
{
    if (str == NULL)
        return str;

    char cur_sym = *str;
    //printf("%s\n", str);
    while (str != NULL && cur_sym != '\0' && cur_sym != ' ' && cur_sym != '\n' && cur_sym != '\t' && cur_sym != EOF)
    {
        safe_write(output_file, str, 1);
        str++;
        if (str != NULL)
            cur_sym = *str;
    }
    return str;
}
BIZZ_BUZZ read_num(const char **str)
{
    if (str == NULL || *str == NULL || **str == '\0' || **str == EOF)
        return END;

    //Tnow everything is ok

    char cur_symbol = **str;
    if (cur_symbol == '-')
    {
        (*str)++;
        if (**str != '\0')
            cur_symbol = **str;
    }

    int del3 = 0, del5 = 0;
    while (cur_symbol >= '0' && cur_symbol <= '9')
    {
        del3 = (del3 + (cur_symbol - '0')) % 3;
        del5 = (cur_symbol - '0') % 5;
        (*str)++;
        if (*str == NULL || **str == '\0')
            return compare_dels(del3, del5);

        cur_symbol = **str;
    }
    if (cur_symbol != '\0' && cur_symbol != EOF && cur_symbol != '\n' && cur_symbol != ' ')
    {
        return LETTERS;
    }

    return compare_dels(del3, del5);
}
BIZZ_BUZZ compare_dels(int del3, int del5)
{
    if (del3 == 0)
    {
        if (del5 == 0)
            return FIFTEEN;
        return THREE;
    }
    if (del5 == 0)
        return FIVE;
    return NOTHING;
}
//------------------------------------------------------

void close_files(fd files)
{
    close(files.input_);
    close(files.output_);
}

//CHECKING FUNCTIONS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++
int ok(int fd_read)
{
    struct stat stat_input;
    fstat(fd_read, &stat_input);

    return (S_ISREG(stat_input.st_mode));
}
void errror(char info[], size_t LINE, const char *FILE)
{
    printf("Mistake was found in %Ilu in file: %s", LINE, FILE);
    printf("\n\nProblem: %s\n\n", info);
    assert(0);
}
void safe_write(int file, const char *ptr, __off_t bytes)
{
    int out_number = write(file, ptr, bytes);
    if (out_number == -1)
        ERROR("Data wasn't written in file properly!");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++
