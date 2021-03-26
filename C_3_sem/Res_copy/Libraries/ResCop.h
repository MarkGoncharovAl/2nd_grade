#pragma once

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "safe_ops.h"

#define MAX_SIZE 50

char *AddAsFolder(const char *folder);

bool_t CheckFolder(const char *from, const char *to);
char *GetFullName(const char *folder, const char *file);

int CreateWatch(fd_t in, const char *folder);

/////////////////////////////////
//!Copying
//!use with GetFullName
void CopyFiles(const char *folder1, const char *file1, const char *folder2, const char *file2);
void CopyFolders(const char *folder1, const char *file1, const char *folder2, const char *file2, fd_t log);
/////////////////////////////////

/////////////////////////////////
//!Deleting
void DeleteFile(const char *folder, char *file);
void DeleteFolder(const char *folder, char *file);
/////////////////////////////////

/////////////////////////////////
//! Processing cases
void ProcessFolder(uint32_t mask, char *name, char *from, char *to, fd_t log);
void ProcessFile(uint32_t mask, char *name, char *from, char *to, fd_t log);
/////////////////////////////////

/////////////////////////////////
//!Adding watches
void SIG_USR2(int);
int AddWatch(char *from, char *to, fd_t log);
/////////////////////////////////

char *CreateOut(char *file);