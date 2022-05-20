#ifndef _CHAT_H
#define _CHAT_H

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "color_output.h"
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "scanbox.h"

#define SA struct sockaddr

void func(int connfd);


#endif
