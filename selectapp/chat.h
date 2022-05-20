#ifndef _CHAT_H
#define _CHAT_H

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>

#include "color_output.h"
#include "scanbox.h"

#define MAX_EVENT_NUMBER 1024 //事件总数量
#define BUFFER_SIZE 10        //缓冲区大小，这里为10个字节
#define ENABLE_ET 0           // ET模式

#define MAX 80
#define SA struct sockaddr

void func(int connfd);


#endif
