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





int SetNonblocking(int fd);
void AddFd(int epoll_fd, int fd, bool enable_et);
void lt_process(struct epoll_event* events, int number, int epoll_fd, int listen_fd);
void et_process(struct epoll_event* events, int number, int epoll_fd, int listen_fd);
void func(int sockfd);


#endif
