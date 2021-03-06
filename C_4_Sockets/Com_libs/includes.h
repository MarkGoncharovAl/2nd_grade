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
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <linux/limits.h>

#include "log.h"
#include "fast_use.h"
#include "../crypto/crypto.h"

#define EXIT1(err) {ret = err; goto exit1;}
#define EXIT2(err) {ret = err; goto exit2;}