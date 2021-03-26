#pragma once
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/un.h>

#include <termios.h>
#include <netinet/in.h>
#include <netinet/ip.h> 

#include <arpa/inet.h>

#include "fcntl.h"
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>

#include "log.h"
#include "fast_use.h"