#include "epolll.h"
#include "color_output.h"
#include "scanbox.h"

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
    strcpy(p->name, nmsg.name); //复制字符
    p->cfd = connfd;
    p->portid = nmsg.port_name;
    p->next = Head->next;
    Head->next = p;
    bzerobuffer(nmsg);
}
/*
//创建节点删除函数
void delete_client(linkList Head, NMSG nmsg, int connfd)
{
    linkList p = (linkList)malloc(sizeof(linklist));
    strcpy(p->name, nmsg.name); //复制字符
    p->cfd = connfd;
    *(p->portid) = nmsg.port_name;
    p->next = Head->next;
    Head->next = p;
}
*/
//判断链表中是否已经存储
int name_exist(linkList H, NMSG nmsg, int connfd)
{
    linkList s = H->next;
    while (s)
    { //如果服务器中已经有这个人了，就不存,并给该客户端发送消息
        printf("%s\n%s\n",s->name,nmsg.name);
        if ((strcmp(s->name, nmsg.name) == 0) && (s->portid == nmsg.port_name))
        { 
            return 0;
        }
        s = s->next;
    }
    return 1;
}



/* 文件描述符设为非阻塞状态
 * 注意：这个设置很重要，否则体现不出高性能
 */
int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}
/*
 * 将文件描述符fd放入到内核中的epoll数据结构中并将fd设置为EPOLLIN可读，同时根据ET开关来决定使用水平触发还是边缘触发模式
 * 注意：默认为水平触发，或上EPOLLET则为边缘触发
 */
void addfd(int epoll_fd, int fd, bool enable_et)
{
    struct epoll_event event; //为当前fd设置事件
    event.data.fd = fd;       //指向当前fd
    event.events = EPOLLIN;   //使得fd可读
    if (enable_et)
    {                            //此处ENable ET设置为0
        event.events |= EPOLLET; //设置为边缘触发
    }
    // epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);  //将fd添加到内核中的epoll实例中 ///// addfd(epoll_fd, sockfd, true);
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1)
    {
        perror("epoll_ctl wrong");
        exit(EXIT_FAILURE);
    }
    setnonblocking(fd); //设为非阻塞模式
}


void accept_client(struct epoll_event *events, int epoll_fd, int listen_fd) //接受客户端
{
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
    int connfd = accept(listen_fd, (struct sockaddr *)&client_address, &client_addrlength); //创建连接并返回文件描述符（实际进行的三次握手过程）
    // char clientname[30] ;
    // long int l = 0;
    getpeername(connfd, (struct sockaddr *)&client_address, &client_addrlength);
    strcpy(nmsg.name, inet_ntoa(client_address.sin_addr)); // 获取ip
    nmsg.port_name = client_address.sin_port;              //获取端口
    printf("IP:%s,PORT%d", nmsg.name, nmsg.port_name);

    addfd(epoll_fd, connfd, false); //添加到epoll结构中并初始化为LT模式
                                    // TODO: 把 connfd 加到自己的数据结构里
    if (name_exist(Head, nmsg, connfd) == 0)
    { //判断在链表结构里面
        bzero(nmsg.text, sizeof(nmsg.text));
        sprintf(nmsg.text, "The user ip:%s port:%d has been registered!\n", nmsg.name, nmsg.port_name);
        send_to_allclient(nmsg);
        bzero(nmsg.text, sizeof(nmsg.text));
    }
    else
    {
        insert_client(Head, nmsg, connfd); //，如果没在链表结构里面，将这个客户端添加到我们的链表中
        printf("加进来了");
        sprintf(nmsg.text, "welcome to join us ip:%s Port:%d!\n", nmsg.name, nmsg.port_name);
        send_to_allclient(nmsg);
        
    }
}

void read_from_stdin(NMSG nmsg)
{
    boxout();
    printf("到了键盘输入的地方\n");
    bzerobuffer(nmsg);
    scantext(nmsg.text, sizeof(nmsg.text));
    printf("nmsg.text:%s\n", nmsg.text); 
}

void read_from_client(NMSG nmsg,  int fd, int epoll_fd)
{
    // printf("%d",evens[i].data.fd);
    int ret = read(fd, nmsg.text, sizeof(nmsg.text));
    if (ret == -1)
    {
        // TODO: 把 这个 fd 从数据结构里移出
        
        if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
        { //删除这个文件描述符
            perror("[epoll_ctl5]");
        }
        close(fd);
    }
    else if (ret == 0)
    {
        printf("Client exit\n");
        // close(events[i].data.fd);
        if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
        { //删除这个文件描述符
            perror("[epoll_ctl3]");
        }
        close(fd);
    }
    else
    {
        newMessage(nmsg);
        printf("在newmassage里面nmsg.text:%s\n",nmsg.text);////这个没问题
        // bzero(buff, BUFFER_SIZE);
    }
}

void send_to_allclient(NMSG nmsg)
{
    int ret = 0;
    linkList t = Head->next; //创建一个临时结点用于遍历
    printf("进来了,send_to_allclient,nmsg.text:%s\n",nmsg.text);
    while (t)
    {
        boxout();
        ret = send(t->cfd, nmsg.text, sizeof(nmsg.text), 0);
        printf("%s\n",nmsg.text);
        if (ret == -1)
        {
            perror("[send_3]");
        }

        t = t->next;
    }
    boxout();
    
    
}

void 
broadcast(NMSG nmsg)
{ //服务器群发消息
    send_to_allclient(nmsg);
    newMessage(nmsg);

}

void bzerobuffer(NMSG nmsg)
{
    bzero(nmsg.text, sizeof(nmsg.text));
    nmsg.port_name = 0;
    //nmsg.cfd, sizeof(nmsg.cfd));
}
/*  LT水平触发
 *  注意：水平触发简单易用，性能不高，适合低并发场合
 * 一旦缓冲区有数据，则会重复不停的进行通知，直至缓冲区数据读写完毕
 */

void lt_process(struct epoll_event *events, int number, int epoll_fd, int server_fd)
{
    int i = 0;
    for (i = 0; i < number; i++)
    {
        int fd = events[i].data.fd;
        // 先处理错误
        if ((events[i].events & EPOLLERR) ||
            (events[i].events & EPOLLHUP) ||
            (!(events[i].events & EPOLLIN)))
        {
            fprintf(stderr, "epoll error at %d, fd: %d\n", i, fd);
            close(fd);
            continue;
        }

        if (fd == server_fd)
        {
            accept_client(events, epoll_fd, server_fd);
            bzerobuffer(nmsg);
        }
        else if (events[i].events & EPOLLIN)
        {
       
            
            if (fd == 0)
            { // 如果收到了来自 STDIN 的输入
                printf("进来了STDIN_FILENO,nmsg.text:%s\n", nmsg.text); ////这个没问题
                read_from_stdin(nmsg);
                
            }
            else
            { // 否则是来自 socket 的输入
                read_from_client(nmsg, fd, epoll_fd);
            }
            
            printf("在广播之前的nmsg.text:::%s\n",nmsg.text);
            broadcast(nmsg);
            bzerobuffer(nmsg);
        }
    }
}
////以后并发多了，可以直接改成ET触发的