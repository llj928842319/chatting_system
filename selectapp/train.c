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


//创建头节点
linkList head_init()
{
    linkList head = (linkList)malloc(sizeof(linklist));
    bzero(head, sizeof(linklist));
    head->next = NULL;
    return head;
}

//创建节点插入函数
void insert_client(linkList Head, NMSG nmsg, int connfd)
{
    linkList p = (linkList)malloc(sizeof(linklist));
    strncpy(p->name, nmsg.name, sizeof(nmsg.name));
    p->cfd = connfd;
    p->next = Head->next;
    Head->next = p;
}

//判断链表中是否已经存储
int name_exist(linkList H ,NMSG nmsg,int connfd)
{
    linkList s = H->next;
    while (s){//如果服务器中已经有这个人了，就不存,并给该客户端发送消息
        if (strncmp(nmsg.name, s->name, sizeof(nmsg.name)) == 0){
            //free(s);
            return 0;
        }
        s=s->next;
    }
    //free(s);
    return 1;
}



int main()
{
    
    insert_client(Head,nmsg,2);
    insert_client(Head,nmsg,4);
    insert_client(Head,nmsg,6);
    insert_client(Head,nmsg,8);
    insert_client(Head,nmsg,10);
    insert_client(Head,nmsg,12);
    insert_client(Head,nmsg,14);
    insert_client(Head,nmsg,16);
    insert_client(Head,nmsg,18);
    insert_client(Head,nmsg,20);
    insert_client(Head,nmsg,21);
    
    linkList t = Head;


    while (t){
        if (!strncmp(nmsg.dst_name, t->name, sizeof(nmsg.dst_name))){//表示有这个人
            
            printf("%d\n",nmsg.cfd);
        }
        t = t->next;
    return 0;
    

    }
}