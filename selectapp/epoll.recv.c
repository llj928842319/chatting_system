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
    // printf("%s\n", nmsg.name);
    // printf("%s\n", nmsg.dst_name);
    strcpy(p->name, nmsg.name); //复制字符
    p->cfd = connfd;
    (p->portid) = nmsg.port_name;
    p->next = Head->next;
    Head->next = p;
}

//判断链表中是否已经存储
int name_exist(linkList H, NMSG nmsg, int connfd)
{
    linkList s = H->next;
    while (s)
    { //如果服务器中已经有这个人了，就不存,并给该客户端发送消息
        if ((strcmp(s->name, nmsg.name) == 0) && ((s->portid) == nmsg.port_name))
        {
            return 0;
        }
        printf("\n\n\n%s %s\n\n\n", s->name, nmsg.name);
        printf("\n\n%d %d", (s->portid), nmsg.port_name);
        // free(s);
        s = s->next;
    }
    // free(s);
    return 1;
}

int dstname_exist(linkList H, NMSG nmsg, int connfd)
{
    linkList t = H->next;
    while (t)
    {
        if (!((t->portid) == nmsg.port_name))
        { //表示有这个人
            // free(t);
            return 1;
        }
        t = t->next;
    }
    // free(t);
    return 0; //表示没有这个人
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


int sendall(NMSG nmsg) //群发消息
{
    int ret = 0;
    linkList t = Head->next; //创建一个临时结点用于遍历
    while (t)
    {
        boxout();
        ret = send(t->cfd, nmsg.text, sizeof(nmsg.text), 0);
        if (ret == -1)
        {
            perror("[send_3]");
        }

        t = t->next;
    }
    boxout();
    bzero(nmsg.text, sizeof(nmsg.text));
    return ret;
}



void accept_client(struct epoll_event *events, int epoll_fd, int listen_fd) //接受客户端
{
    int sendret = 0;

    static int i = 0;
    i++;
    printf("调用i次:%d\n",i);
    
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
    int connfd = accept(listen_fd, (struct sockaddr *)&client_address, &client_addrlength); //创建连接并返回文件描述符（实际进行的三次握手过程）
    // char clientname[30] ;
    // long int l = 0;
    getpeername(connfd, (struct sockaddr *)&client_address, &client_addrlength);
    strcpy(nmsg.name, inet_ntoa(client_address.sin_addr)); // 获取ip
    nmsg.port_name = client_address.sin_port;              //获取端口

    addfd(epoll_fd, connfd, false); //添加到epoll结构中并初始化为LT模式
                                    // TODO: 把 connfd 加到自己的数据结构里
    if (name_exist(Head, nmsg, connfd) == 0)
    { //判断在链表结构里面
        bzero(nmsg.text, sizeof(nmsg.text));
        sprintf(nmsg.text, "The user ip:%s port:%d has been registered!\n", nmsg.name, nmsg.port_name);
        if (send(connfd, &nmsg.text, sizeof(nmsg.text), 0) == -1)
        {
            perror("[send_4]");
        }
        bzero(nmsg.text, sizeof(nmsg.text));
    }
    else
    {
        insert_client(Head, nmsg, listen_fd); //，如果没在链表结构里面，将这个客户端添加到我们的链表中

        sprintf(nmsg.text, "welcome to join us ip:%s Port:%d!\n", nmsg.name, nmsg.port_name);
        sendret = sendall(nmsg);
        // free(t);
        if (sendret == -1)
        {
            perror("send error");
        }
    }
}


/*  LT水平触发
 *  注意：水平触发简单易用，性能不高，适合低并发场合
 *        一旦缓冲区有数据，则会重复不停的进行通知，直至缓冲区数据读写完毕
 */

void lt_process(struct epoll_event *events, int number, int epoll_fd, int listen_fd)
{
    int i = 0;

    for (i = 0; i < number; i++)
    { //已经就绪的事件，这些时间可读或者可写

        if (events[i].data.fd == listen_fd)
        { //如果监听类型的描述符，则代表有新的client接入，则将其添加到内核中的epoll结构中
            accept_client(events, epoll_fd, listen_fd);
        }

        else if (events[i].events == EPOLLIN)
        { //有读数据，从网络读取，从键盘读取

            if (events[i].data.fd == 0)
            { //群聊，发送给每一个人///进来了，也发出去了

                boxout();
                bzero(nmsg.text, sizeof(nmsg.text));
                scantext(nmsg.text, BUFFER_SIZE);
                newMessage(nmsg);
                if (sendall(nmsg) == -1)
                    break;
            }
            // printf("%d",evens[i].data.fd);
            int ret = read(events[i].data.fd, nmsg.text, sizeof(nmsg.text));
            if (ret == -1)
            {
                // TODO: 把 这个 fd 从数据结构里移出
                // close(events[i].data.fd);
                if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) == -1)
                { //删除这个文件描述符
                    perror("[epoll_ctl5]");
                    continue; //进行处理下一个事件
                }
                close(events[i].data.fd);
            }
            else if (ret == 0)
            {
                printf("Client exit\n");
                // close(events[i].data.fd);
                if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) == -1)
                { //删除这个文件描述符
                    perror("[epoll_ctl3]");

                    continue; //进行处理下一个事件
                }
                close(events[i].data.fd);
            }
            else
            {
                newMessage(nmsg);
                boxout();

                // bzero(buff, BUFFER_SIZE);
            }
            bzero(nmsg.text, sizeof(nmsg.text));

            // buff[ret - 1] = '\0';
        }
        /*
        else{//当设置了ctl_wait(),一直等的时候，这个不会调用

            printf("something unexpected happened!\n");
        }
        */
    }
}

////以后并发多了，可以直接改成ET触发的

/*  ET Work mode features: efficient but potentially dangerous */
/*  ET边缘触发
 *  注意：边缘触发由于内核不会频繁通知，所以高效，适合高并发场合，但是处理不当将会导致严重事故
          其通知机制和触发方式参见之前讲解，由于不会重复触发，所以需要处理好缓冲区中的数据，避免脏读脏写或者数据丢失等
 */
/*
void et_process(struct epoll_event* events, int number, int epoll_fd, int listen_fd)
{
    char buff[BUFFER_SIZE];
    int i;
    for(i = 0; i < number; i++)
    {

        if(events[i].data.fd == listen_fd) //如果有新客户端请求过来，将其添加到内核中的epoll结构中并默认置为ET模式
        {
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof(client_address);
            int connfd = accept(listen_fd, (struct sockaddr*)&client_address, &client_addrlength);
            addfd(epoll_fd, connfd, true);
        }
        else if(events[i].events & EPOLLIN) //如果客户端有数据过来
        {
        // read the message from client and copy int in buffer

            bzero(buff, BUFFER_SIZE);
            int ret = read(events[i].data.fd, buff, sizeof(buff));
            if (ret == 0)
            {
                printf("Client exit\n");
                close(events[i].data.fd);
                break;
            }
            buff[ret - 1] = '\0';
            newMessage("client", buff);
            boxout();
         //   for(int j = 0; j < number; j++){
         //       epoll_ctl(epoll_fd, EPOLL_CTL_MOD, 1, events);
          //  }
        }
        else if (events[i].data.fd & EPOLLOUT){
            printf("收到键盘的输入\n");
            boxout();
            bzero(buff, BUFFER_SIZE);
            scantext(buff, BUFFER_SIZE);
            write(events[i].data.fd, buff, sizeof(buff));//这个地方是自己的消息也打印了
            //再加上群发多人如何处理
            for(int j = 0; j < number; j++){
           //     if(events[i].data.fd & EPOLLOUT ){
                write(events[j].data.fd, buff, sizeof(buff));
              //  epoll_ctl(epoll_fd, EPOLL_CTL_MOD, 0, events);
             //   }
            }

         }
        else{

            printf("something unexpected happened!\n");
        }
    }
}

*/