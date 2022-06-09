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


#define MAX_EVENT_NUMBER 1024 //事件总数量
#define BUFFER_SIZE 200       //缓冲区大小，这里为10个字节
#define ENABLE_ET 0           // ET模式
#define SA struct sockaddr

//定义结构体
typedef struct newmessager{
    char name[32];//客户端姓名 也就是IP
    uint16_t port_name;//端口号
    int cfd;//对应的套接字
    char text[BUFFER_SIZE];//消息内容，也就是缓存区 

}NMSG, *NMSG_PTR;
extern NMSG_PTR nmsg;//实例化


//定义链表，存储客户端，
typedef struct  client_list
{ 
    char name[32];//客户端名字
    uint16_t portid;//端口
    int cfd;//套接字
    struct client_list *next;
}linklist, *linkList;

extern linkList Head;//头头



linkList head_init();
void NMSG_init();
void NMSG_delete();
int name_exist(linkList H, NMSG_PTR nmsg, int connfd);
//int dstname_exist(linkList H, NMSG nmsg, int connfd);
void insert_client(linkList Head, NMSG_PTR nmsg, int connfd);
int setnonblocking(int fd);
void addfd(int epoll_fd, int fd, bool enable_et);
void accept_client(struct epoll_event* events, int epoll_fd, int listen_fd);
void read_from_stdin(NMSG_PTR nmsg);
void read_from_client(NMSG_PTR nmsg, int fd, int epoll_fd);
void send_to_allclient(NMSG_PTR nmsg);
void broadcast(NMSG_PTR nmsg);
void lt_process(struct epoll_event* events, int number, int epoll_fd, int listen_fd );

void bzerobuffer(NMSG_PTR nmsg);

//void et_process(struct epoll_event* events, int number, int epoll_fd, int listen_fd);  //给以后ET触发留的

#endif