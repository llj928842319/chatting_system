#include "epolll.h"

//创建头节点
linkList head_init()
{
    linkList head = (linkList)malloc(sizeof(linklist));
    bzero(head, sizeof(linklist));
    head->next = NULL;
    return head;
}

//创建节点插入函数
void insert_client(linkList Head, NMSG_PTR nmsg, int connfd)
{
    linkList p = (linkList)malloc(sizeof(linklist));
    strncpy(p->name, nmsg->name, sizeof(nmsg->name));
    p->cfd = connfd;
    p->next = Head->next;
    Head->next = p;
}

//判断链表中是否已经存储
int name_exist(linkList H ,NMSG_PTR nmsg,int connfd)
{
    linkList s = H->next;
    while (s){//如果服务器中已经有这个人了，就不存,并给该客户端发送消息
        if (strncmp(nmsg->name, s->name, sizeof(nmsg->name)) == 0){
            free(s);
            return 0;
        }
        s=s->next;
    }
    free(s);
    return 1;
}

int dstname_exist(linkList H,NMSG_PTR nmsg,int connfd)
{
    linkList t=H->next;
    while (t){
        if (!strncmp(nmsg->dst_name, t->name, sizeof(nmsg->dst_name))){//表示有这个人
            free(t);
            return 1;
        }
        t = t->next;
    }
    free(t);
    return 0;//表示没有这个人
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
    if (enable_et){           //此处ENable ET设置为0
        event.events |= EPOLLET; //设置为边缘触发
    }
    // epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);  //将fd添加到内核中的epoll实例中 ///// addfd(epoll_fd, sockfd, true); 
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1){
        perror("epoll_ctl wrong");
        exit(EXIT_FAILURE);
    }
    setnonblocking(fd); //设为非阻塞模式
}




/*  LT水平触发 
 *  注意：水平触发简单易用，性能不高，适合低并发场合
 *        一旦缓冲区有数据，则会重复不停的进行通知，直至缓冲区数据读写完毕
 */

void lt_process(struct epoll_event* events, int number, int epoll_fd, int listen_fd )
{
    
    
    int i;

    for(i = 0; i < number; i++){ //已经就绪的事件，这些时间可读或者可写
    
        if(events[i].data.fd == listen_fd){  //如果监听类型的描述符，则代表有新的client接入，则将其添加到内核中的epoll结构中
        
            
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof(client_address);
            int connfd = accept(listen_fd, (struct sockaddr*)&client_address, &client_addrlength); //创建连接并返回文件描述符（实际进行的三次握手过程）
            addfd(epoll_fd, connfd, false);  //添加到epoll结构中并初始化为LT模式
            // TODO: 把 connfd 加到自己的数据结构里
            if (name_exist(Head, nmsg, connfd)==0){
                bzero(nmsg->text, sizeof(nmsg->text));
                sprintf(nmsg->text,"The user %s has been registered!\n",nmsg->name);
                if (send(connfd, &nmsg, sizeof(nmsg), 0) == -1){
                    perror("[send_4]");
                    
                }
                bzero(nmsg->text, sizeof(nmsg->text));
                
            } 
            else{
                insert_client(Head, nmsg, connfd);//，如果没在链表结构里面，将这个客户端添加到我们的链表中
                
                linkList q = Head->next;      //新建一个暂时的链表用来遍历
                sprintf(nmsg->text, "welcome to join us %s!\n",nmsg->name);
                while (q){
                    if (send(q->cfd, &nmsg->text, sizeof(nmsg->text), 0) == -1){
                        perror("[send_1]");
                    }
                    q = q->next;                   
                }
                free(q);
                bzero(nmsg->text,sizeof(nmsg->text));
                

         }
        }
        else if(events[i].events == EPOLLIN){   //如果客户端有数据过来
            if (events[i].data.fd == 0){//群聊，发送给每一个人///进来了，也发出去了
              
                boxout();
                scantext(nmsg->text, BUFFER_SIZE);
                newMessage("server", nmsg->text);
                
                
                //strncpy(nmsg->text, buff, sizeof(nmsg->name));为什么服务端自己不打印了呢
                
                linkList t = Head->next;//创建一个临时结点用于遍历
                while (t){                  
                if (send(t->cfd, nmsg->text, sizeof(nmsg->text),0)==-1){
                    perror("[send_3]");
                    break;
                    }
                    
                    t=t->next;
                }
                boxout();
                bzero(nmsg->text, sizeof(nmsg->text));
                free(t);
                break;
            }
            if(events[i].data.fd == 5){
                printf("%d",events[i].data.fd);
                newMessage("client", nmsg->text);
                boxout();
            }
                       //bzero(buff, BUFFER_SIZE);
            int ret = read(events[i].data.fd, nmsg->text, sizeof(nmsg->text));
            if (ret == 0) {
                printf("Client exit\n");   
                    // TODO: 把 这个 fd 从数据结构里移出
                   // close(events[i].data.fd);
                if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL)==-1){//删除这个文件描述符
                    perror("[epoll_ctl3]");
                    
                    continue;//进行处理下一个事件
                    }
            }
            
                      
                
              // buff[ret - 1] = '\0';            
        }   
        else{//当设置了ctl_wait(),一直等的时候，这个不会调用
        
            printf("something unexpected happened!\n");
        }
    

    }   

}
