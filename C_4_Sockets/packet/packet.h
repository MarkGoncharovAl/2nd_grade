#pragma once
#include "../Com_libs/includes.h"
#include "../Com_libs/const.h"

typedef struct
{
    size_t size_;
    char* data_;
    int name_;
} M_pack_named;

typedef struct
{
    size_t size_;
    char* data_;
} M_pack_unnamed;

M_pack_named* M_CreatePack_Named (const char* data , size_t size , int name);
M_pack_unnamed* M_CreatePack_Unnamed (const char* data , size_t size);

//not copying -> should be controlled memory! 
M_pack_named* M_CreatePack_Named_Mem (char* data , size_t size , int name);
M_pack_unnamed* M_CreatePack_Unnamed_Mem (char* data , size_t size);

void M_DestroyPack_Named (M_pack_named* packet);
void M_DestroyPack_Unnamed (M_pack_unnamed* packet);

M_pack_unnamed* M_RecoverPack (M_pack_named* pack);



//!returns dynamic memory
M_pack_unnamed* M_ReadPack_Unnamed (int fd);
//!return -1 if error
int M_WritePack_Unnamed (int fd , M_pack_unnamed* pack);



//!returns dynamic memory
M_pack_named* M_ReadPack_Named (int fd , struct sockaddr* addr);
//!return -1 if error
int M_WritePack_Named (int fd , struct sockaddr* addr , M_pack_named* pack);

M_pack_named* M_CreatePack_STATIC (char* data , size_t size , int name);