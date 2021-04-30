#pragma once

#include <stdio.h>
#include <ctype.h>
#include "../Com_libs/log.h"

//#define CAESAR

enum {ENCRYPT = 0, DECRYPT};
int M_EncryptString(char* str, size_t size);
int M_DecryptString(char* str, size_t size);