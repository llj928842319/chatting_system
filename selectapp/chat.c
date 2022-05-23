
#include "chat.h"
//这个代码是epoll实现多并发这一套的

#define MAX_EVENT_NUMBER 1024 //事件总数量
#define BUFFER_SIZE 10       //缓冲区大小，这里为10个字节
#define ENABLE_ET 0           // ET模式


#define SA struct sockaddr



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
    {
        event.events |= EPOLLET; //设置为边缘触发
    }
    // epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);  //将fd添加到内核中的epoll实例中
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1)
    {
        perror("epoll_ctl wrong");
        exit(EXIT_FAILURE);
    }
    setnonblocking(fd); //设为非阻塞模式
}





/*  LT水平触发 
 *  注意：水平触发简单易用，性能不高，适合低并发场合
 *        一旦缓冲区有数据，则会重复不停的进行通知，直至缓冲区数据读写完毕
 */

void lt_process(struct epoll_event* events, int number, int epoll_fd, int listen_fd)
{
    char buff[BUFFER_SIZE];
    int i;
    for(i = 0; i < number; i++) //已经就绪的事件，这些时间可读或者可写
    {
        int sockfd = events[i].data.fd; //获取描述符
        if(sockfd == listen_fd)  //如果监听类型的描述符，则代表有新的client接入，则将其添加到内核中的epoll结构中
        {
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof(client_address);
            int connfd = accept(listen_fd, (struct sockaddr*)&client_address, &client_addrlength); //创建连接并返回文件描述符（实际进行的三次握手过程）
            addfd(epoll_fd, connfd, false);  //添加到epoll结构中并初始化为LT模式
        }
        else if(events[i].events & EPOLLIN) //如果客户端有数据过来
        {
            if (events[i].data.fd == sockfd)
                {
                    // read the message from client and copy int in buffer

                    bzero(buff, BUFFER_SIZE);
                    int ret = read(sockfd, buff, sizeof(buff));
                    if (ret == 0)
                    {
                        printf("Client exit\n");
                        close(sockfd);
                        break;
                    }
                    buff[ret - 1] = '\0';
                    newMessage("client", buff);
                    boxout();
                }
        }
        else if (events[i].data.fd == 0)
        {
            printf("收到键盘的输入\n");
            boxout();
            bzero(buff, BUFFER_SIZE);
            scantext(buff, BUFFER_SIZE);
            //////群发多人
            //write(connfd, buff, sizeof(buff));
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
    char buff[BUFFER_SIZE];
    int i;
    for(i = 0; i < number; i++)
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listen_fd) //如果有新客户端请求过来，将其添加到内核中的epoll结构中并默认置为ET模式
        {
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof(client_address);
            int connfd = accept(listen_fd, (struct sockaddr*)&client_address, &client_addrlength);
            addfd(epoll_fd, connfd, true); 
        }
        else if(events[i].events & EPOLLIN) //如果客户端有数据过来
        {

            if (events[i].data.fd == sockfd)
            {
             // read the message from client and copy int in buffer

                bzero(buff, BUFFER_SIZE);
                int ret = read(sockfd, buff, sizeof(buff));
                if (ret == 0)
                {
                    printf("Client exit\n");
                    close(sockfd);
                    break;
                }
                buff[ret - 1] = '\0';
                newMessage("client", buff);
                boxout();
            }    
        }
         else if (events[i].data.fd == 0){
            
            boxout();
            bzero(buff, BUFFER_SIZE);
            scantext(buff, BUFFER_SIZE);
            write(sockfd, buff, sizeof(buff));
         }
        else{
        
            printf("something unexpected happened!\n");
        }
    }
}



// function designed for chat between client and server
void func(int sockfd)
{
    struct epoll_event events[MAX_EVENT_NUMBER];
    int epoll_fd = epoll_create(5); //在内核中创建epoll实例，flag为5只是为了分配空间用，实际可以不用带
    if (epoll_fd == -1)
    {
        perror("fail to create epoll!\n");
            exit(1);
    }


    addfd(epoll_fd, sockfd, true); //添加文件描述符到epoll对象中      接受对方文件
    addfd(epoll_fd, 0, true);
    if(epoll_fd == -1)
    {
        perror("fail to create epoll!\n");
        exit(EXIT_FAILURE);
    }
       
    
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
            et_process(events, ret, epoll_fd, sockfd);
        }
        else  //LT处理方式
        {
            lt_process(events, ret, epoll_fd, sockfd);
        }
    }
    
}


/*
        connfd = accept(sockfd, (SA *)&cli, &len);
        if (connfd < 0)
        {
            perror("server accept failed..\n");
            exit(0);
        }
        printf("%d\n", connfd);
        printf("server accept the client...\n");
        boxout();
*/

/*
        for (;;)
        {
            nfds = epoll_wait(epoll_fd, events, MAX_EVENT_NUMBER, -1);
            if (nfds == -1)
            {
                perror("epoll_wait");
                exit(EXIT_FAILURE);
            }

            for (n = 0; n < nfds; ++n)
            {

                if (events[n].data.fd == connfd)
                {
                    // read the message from client and copy int in buffer

                    bzero(buff, MAX);
                    int ret = read(connfd, buff, sizeof(buff));
                    if (ret == 0)
                    {
                        printf("Client exit\n");
                        close(connfd);
                        break;
                    }
                    buff[ret - 1] = '\0';
                    newMessage("client", buff);
                    boxout();
                }
                else if (events[n].data.fd == 0)
                {
                    boxout();
                    bzero(buff, MAX);
                    scantext(buff, MAX);
                    write(connfd, buff, sizeof(buff));
                }
                else
                {
                }
            }
        break;  
        }
        */       
        


/*
    //inifinite loop for chat
    fd_set read_fds;
    boxout();
    for (;;)
    {
        FD_ZERO( &read_fds);

        FD_SET(0, &read_fds);
        FD_SET(connfd, &read_fds);
        char buff[MAX] = {0};

        bzero(buff,MAX);

        int select_ready = select(connfd+1, &read_fds, NULL, NULL, NULL);
        if (select_ready < 0 ) {
            perror("select error\n");
            exit(0);
        }

        //int n = 0;
        if (FD_ISSET(0, &read_fds)) {   //当 stdin 的缓存中 有数据的时候，则按以下进行处理
            //n = 0;

            // while ( (n < MAX) && (buff[n++] = getchar()) != '\n')
            // {
            // 	if (n == 1)
            // 		scantext();
            // }
            boxout();
            scantext(buff, MAX);
            write(connfd, buff, sizeof(buff));
        }

        if (FD_ISSET(connfd, &read_fds)) {
            //read the message from client and copy int in buffer
            int ret = read(connfd, buff, sizeof(buff));
            if (ret == 0) {
                printf("Client exit\n");
            return;
            }
            //print buffer which contains the client contents
            //printf("from client: %s\t to client:", buff);

            buff[ret-1] = '\0';
            newMessage("client", buff);
            bzero(buff, MAX);
        }
    } */
