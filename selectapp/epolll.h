#ifndef _EPOLLL_H
#define _EPOLLL_H

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
#include <fcntl.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>


#include "color_output.h"
#include "scanbox.h"



#define MAX_EVENT_NUMBER 1024 //事件总数量
#define BUFFER_SIZE 200       //缓冲区大小，这里为10个字节
#define ENABLE_ET 0           // ET模式
#define SA struct sockaddr

//定义结构体
typedef struct newmessager{
    char name[30];//客户端姓名
    int cfd;//对应的套接字
    char type;//消息类型
    char dst_name[30];//目的客户姓名
    char text[BUFFER_SIZE];//消息内容，也就是缓存区 
}NMSG;
NMSG nmsg;//实例化


//定义链表，存储客户端，
typedef struct  client_list
{
    char name[30];//客户端名字
    int cfd;//套接字
    struct client_list *next;
}linklist, *linkList;

linkList Head;//头头


linkList head_init();
int name_exist(linkList H, NMSG nmsg, int connfd);
int dstname_exist(linkList H, NMSG nmsg, int connfd);
int setnonblocking(int fd);
void addfd(int epoll_fd, int fd, bool enable_et);
void lt_process(struct epoll_event* events, int number, int epoll_fd, int listen_fd );
//void et_process(struct epoll_event* events, int number, int epoll_fd, int listen_fd);  //给以后ET触发留的

#endif