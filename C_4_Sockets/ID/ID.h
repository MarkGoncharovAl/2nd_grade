#pragma once
#include "../Com_libs/includes.h"
#include "../Com_libs/const.h"
#include "../packet/packet.h"

//returns new IP
int M_AddID(int fd);

//returns file descriptor
void M_DeleteID(int ID);

//returns -1 if client wasn't found
int M_GetFD_FromID(int ID);

int M_Close_IDS(int sk);