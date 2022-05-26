
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#define MAX_EVENT_NUMBER 1024   //事件总数量
#define BUFFER_SIZE 10          //缓冲区大小，这里为10个字节
#define ENABLE_ET 0             //ET模式
/* 文件描述符设为非阻塞状态
 * 注意：这个设置很重要，否则体现不出高性能
 */
int SetNonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}
/* 将文件描述符fd放入到内核中的epoll数据结构中并将fd设置为EPOLLIN可读，同时
根据ET开关来决定使用水平触发还是边缘触发模式 
 * 注意：默认为水平触发，或上EPOLLET则为边缘触发
*/
void AddFd(int epoll_fd, int fd, bool enable_et)
{
    struct epoll_event event;  //为当前fd设置事件
    event.data.fd = fd;        //指向当前fd
    event.events = EPOLLIN;    //使得fd可读
    if(enable_et)
    {
        event.events |= EPOLLET; //设置为边缘触发
    }
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);  //将fd添加到内核中的epoll实例中
    SetNonblocking(fd);  //设为非阻塞模式                      
}


/*  LT水平触发 
 *  注意：水平触发简单易用，性能不高，适合低并发场合
 *        一旦缓冲区有数据，则会重复不停的进行通知，直至缓冲区数据读写完毕
 */

void lt_process(struct epoll_event* events, int number, int epoll_fd, int listen_fd)
{
    char buf[BUFFER_SIZE];
    int i;
    for(i = 0; i < number; i++) //已经就绪的事件，这些时间可读或者可写
    {
        int sockfd = events[i].data.fd; //获取描述符
        if(sockfd == listen_fd)  //如果监听类型的描述符，则代表有新的client接入，则将其添加到内核中的epoll结构中
        {
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof(client_address);
            int connfd = accept(listen_fd, (struct sockaddr*)&client_address, &client_addrlength); //创建连接并返回文件描述符（实际进行的三次握手过程）
            AddFd(epoll_fd, connfd, false);  //添加到epoll结构中并初始化为LT模式
        }
        else if(events[i].events & EPOLLIN) //如果客户端有数据过来
        {
            printf("-->LT Mode: it was triggered once!\n");
            memset(buf, 0, BUFFER_SIZE); 
            int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
            if(ret <= 0)  //读取数据完毕后，关闭当前描述符
            {
                close(sockfd);
                continue;
            }
            printf("get %d bytes of content: %s\n", ret, buf);
        }
        else
        {
            printf("something unexpected happened!\n");
        }
    }
}
/*  ET Work mode features: efficient but potentially dangerous */
/*  ET边缘触发
 *  注意：边缘触发由于内核不会频繁通知，所以高效，适合高并发场合，但是处理不当将会导致严重事故
          其通知机制和触发方式参见之前讲解，由于不会重复触发，所以需要处理好缓冲区中的数据，避免脏读脏写或者数据丢失等
 */
void et_process(struct epoll_event* events, int number, int epoll_fd, int listen_fd)
{
    char buf[BUFFER_SIZE];
    int i;
    for(i = 0; i < number; i++)
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listen_fd) //如果有新客户端请求过来，将其添加到内核中的epoll结构中并默认置为ET模式
        {
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof(client_address);
            int connfd = accept(listen_fd, (struct sockaddr*)&client_address, &client_addrlength);
            AddFd(epoll_fd, connfd, true); 
        }
        else if(events[i].events & EPOLLIN) //如果客户端有数据过来
        {
            printf("-->ET Mode: it was triggered once\n");
            while(1) //循环等待
            {
                memset(buf, 0, BUFFER_SIZE);
                int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
                if(ret < 0)
                {
                    if(errno == EAGAIN || errno == EWOULDBLOCK) //通过EAGAIN检测，确认数据读取完毕
                    {
                        printf("-->wait to read!\n");
                        break;
                    }
                    close(sockfd);
                    break;
                }
                else if(ret == 0) //数据读取完毕，关闭描述符
                {
                    close(sockfd);
                }
                else //数据未读取完毕，继续读取
                {
                    printf("get %d bytes of content: %s\n", ret, buf);
                }
            }
        }
        else
        {
            printf("something unexpected happened!\n");
        }
    }
}
int main(int argc, char* argv[])
{
    const char* ip = "10.0.76.135";
    int port = 9999;
    
    //套接字设置这块，参见https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html
    int ret = -1;
    struct sockaddr_in address; 
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    int listen_fd = socket(PF_INET, SOCK_STREAM, 0);    //创建套接字并返回描述符
    if(listen_fd < 0)
    {
        printf("fail to create socket!\n");
        return -1;
    }
    ret = bind(listen_fd, (struct sockaddr*)&address, sizeof(address)); //绑定本机
    if(ret == -1)
    {
        printf("fail to bind socket!\n");
        return -1;
    }
    ret = listen(listen_fd, 5); //在端口上监听
    if(ret == -1)
    {
        printf("fail to listen socket!\n");
        return -1;
    }



    struct epoll_event events[MAX_EVENT_NUMBER];
    int epoll_fd = epoll_create(5);  //在内核中创建epoll实例，flag为5只是为了分配空间用，实际可以不用带
    if(epoll_fd == -1)
    {
        printf("fail to create epoll!\n");
        return -1;
    }
    AddFd(epoll_fd, listen_fd, true); //添加文件描述符到epoll对象中


    
    while(1)
    {
        int ret = epoll_wait(epoll_fd, events, MAX_EVENT_NUMBER, -1); //拿出就绪的文件描述符并进行处理
        if(ret < 0)
        {
            printf("epoll failure!\n");
            break;
        }
        if(ENABLE_ET) //ET处理方式
        {
            et_process(events, ret, epoll_fd, listen_fd);
        }
        else  //LT处理方式
        {
            lt_process(events, ret, epoll_fd, listen_fd);
        }
    }
    close(listen_fd); //退出监听
    return 0;
}