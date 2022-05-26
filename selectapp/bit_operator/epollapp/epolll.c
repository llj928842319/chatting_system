#include "epolll.h"
#define MAX 1024
//定义消息结构体
typedef struct message{
    char type;//消息类型
    char name[30];//客户端姓名
    char dst_name[30];//目的客户姓名
    char text[MAX];//消息内容
}MSG;
MSG msg;//定义全局消息变量进行消息的接收和发送
//定义存储客户端信息链表
typedef struct  client_list{
    char name[30];//客户端的姓名
    int cfd;//对应的在树上的套接字
    struct client_list *next;
}linklist,*linkList ;
//定义头节点，设置为全局变量
linkList H;

//创建头节点函数
linkList head_init(){
  linkList h=(linkList)malloc(sizeof(linklist));
  bzero(h,sizeof(linklist));
  h->next=NULL;
  return h;
}

//创建结点插入函数
void insert_client(linkList H,MSG msg,int cfd){
    linkList p=(linkList)malloc(sizeof(linklist));
    bzero(p,sizeof(linklist));
    strncpy(p->name,msg.name,sizeof(msg.name));
    p->cfd=cfd;
    p->next=H->next;
    H->next=p;
}

int listenfd_init(int port){
    int lfd;

    struct sockaddr_in service;
    bzero(&service,sizeof(service));//清空
    service.sin_family=AF_INET;//初始化
    service.sin_port=htons(port);
    service.sin_addr.s_addr=htonl(INADDR_ANY);

    if ((lfd=socket(AF_INET,SOCK_STREAM,0))==-1){//确定TCP连接
        perror("[socket]");
        return -1;
    }

    int opt=1;//使用端口复用
    if (setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))==-1){
        perror("[setsockopt]");
        return -1;
    }

    //绑定地址结构
    if (bind(lfd,(struct sockaddr *)&service,sizeof(service))==-1){
        perror("[bind]");
        return -1;
    }

    //设置最大监听数
    if (listen(lfd,128)==-1){
        perror("[listen]");
        return -1;
    }
    return lfd;
}

//客户端连接函数
int connectfd_init(int lfd){
    int cfd;
    struct sockaddr_in client;
    bzero(&client,sizeof(client));
    socklen_t len=sizeof(client);

    if ((cfd=accept(lfd,(struct sockaddr *)&client,&len))==-1){
        perror("[accpet]");
        return -1;
    }
    printf("Have a brother join to us!\n");
    return cfd;
}

int name_exist(linkList H ,MSG msg,int cfd){
    linkList s=H->next;
    while (s){//如果服务器中已经有这个人了，就不存,并给该客户端发送消息
        if (strncmp(msg.name,s->name,sizeof(msg.name))==0){
            return 0;
        }
        s=s->next;
    }
    return 1;
}

int dstname_exist(linkList H,MSG msg,int cfd){
    linkList t=H->next;
    while (t){
        if (!strncmp(msg.dst_name,t->name,sizeof(msg.dst_name))){//表示有这个人
            return 1;
        }
        t=t->next;
    }
    return 0;//表示没有这个人
}

int message_handler(int cfd){
    bzero(&msg,sizeof(msg));//清空
    int ret;
    ret=recv(cfd,&msg,sizeof(msg),0);
    if (ret==-1){
        perror("[recv]");
        return -1;
    }
    if (ret==0){//客户端断开连接
        printf("Has a brother disconnect!\n");
        return 0;
    }
    //对消息进行处理
    if (msg.type=='1'){//注册消息
        if (name_exist(H,msg,cfd)==0){
            bzero(msg.text,sizeof(msg.text));
            sprintf(msg.text,"The user %s has been registered!\n",msg.name);
            if (send(cfd,&msg,sizeof(msg),0)==-1){
                perror("[send_4]");
                return -1;
            }
        } else{
            insert_client(H,msg,cfd);//将这个客户添加到我们的链表中
            linkList q=H->next;
            sprintf(msg.text,"welcome to join us %s!\n",msg.name);
            while (q){
                if (send(q->cfd,&msg,sizeof(msg),0)==-1){
                    perror("[send_1]");
                }
                q=q->next;
            }
        }

    }
    if (msg.type=='2'){//私聊消息，进行转发
        //进行检查该用户是否存在
        if (dstname_exist(H,msg,cfd)){//有这个人，将消息发送给这个人
            linkList p=H->next;//指向头节点
            while (p){
                if (!strncmp(p->name,msg.dst_name,sizeof(msg.dst_name))){
                    if (send(p->cfd,&msg,sizeof(msg),0)==-1){
                        perror("[send_2]");
                        break;
                    }
                    printf("OK\n");
                }
                p=p->next;
            }
        } else{
            bzero(msg.text,sizeof(msg.text));
            sprintf(msg.text,"%s has been offline!\n",msg.dst_name);
            if (send(cfd,&msg,sizeof(msg),0)==-1){
                perror("[send_5]");
                return -1;
            }
        }
    }
    if (msg.type=='3'){//群聊，发送给每一个人
        linkList t=H->next;//创建一个临时结点用于遍历
        while (t){
            if (send(t->cfd,&msg,sizeof(msg),0)==-1){
                perror("[send_3]");
                break;
            }
            t=t->next;
        }
    }
    if (msg.type=='4'){//打印在线人员信息
        linkList u=H->next;
        while (u){
            bzero(msg.text,sizeof(msg.text));
            sprintf(msg.text,"%s is on line!\n",u->name);
            if (send(cfd,&msg,sizeof(msg),0)==-1){
                perror("[send_6]");
                return -1;
            }
            u=u->next;
        }
    }
}

void epoll_init(int lfd){
    int ret,ret_r;//用于接收epoll_wait的返回值，表示事件的个数,第二个表示接收消息的返回值
    int cfd;//表示用于客户端连接的套接字
    struct epoll_event ev;
    struct epoll_event events[20];//设置监听事件，和用于存储发生事件的表
    ev.data.fd=lfd;//初始化
    ev.events=EPOLLIN;//读事件

    int epfd;//设置存放套接字的红黑树

    if ((epfd=epoll_create(20))==-1){//表示监听的最大的数量
        perror("[epoll_create]");
        return;
    }

    if (epoll_ctl(epfd,EPOLL_CTL_ADD,lfd,&ev)!=0){//将lfd添加上红黑树上，指定监听的事件为读事件
        perror("[epoll_ctl]");
        return;
    }

    printf("Listening.......\n");
    while (1){//进行事件的监听
        ret=epoll_wait(epfd, events, MAX, -1);//timeout表示超时时长
        if (ret==-1){
            perror("[epoll_wait]");
            break;
        }
        for (int i = 0; i <ret ; ++i) {//循环处理事件
            if (!events[i].events & EPOLLIN){//如果不是读事件
                continue;
            }
            if (events[i].data.fd==lfd){//如果是读事件，且需要读的是lfd，表示的是需要进行连接
                cfd=connectfd_init(lfd);
                if (cfd==-1){
                    return;
                }
                ev.data.fd=cfd;//初始化事件，将cfd对应的监听事件添加到监听对象中
                ev.events=EPOLLIN;
                if (epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,&ev)==-1){//将结点cfd添加上树上
                    perror("[epoll_ctl2]");
                    return;
                }
            } else{//其他套接字的读事件
                ret_r=message_handler(events[i].data.fd);
                if (ret_r==-1){
                    break;//出错直接跳出
                }
                if (ret_r==0){//客户端退出
                    if (epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,NULL)==-1){//删除这个结点，从树上
                        perror("[epoll_ctl3]");
                        continue;//进行处理下一个事件
                    }
                }
            }
        }
    }
}

int main(int argc ,char *argv[]){
    int lfd;//进行套接字的初始化
    lfd=listenfd_init(8888);
    if (lfd==-1){
        return -1;
    }
    H=head_init();
    //进行epoll的连接初始化
    epoll_init(lfd);
    close(lfd);//关闭套接字
    return 0;
}
