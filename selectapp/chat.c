
#include "chat.h"
#include "epolll.h"
//这个代码是epoll实现多并发这一套的

// function designed for chat between client and server
void func(int sockfd)
{
    struct epoll_event events[MAX_EVENT_NUMBER];
    int epfd = epoll_create(20); //在内核中创建epoll实例，flag为5只是为了分配空间用，实际可以不用带
    if (epfd == -1)
    {
        perror("fail to create epoll!\n");
            exit(1);
    }
    Head = head_init();

    addfd(epfd, sockfd, false); //添加文件描述符到epoll对象中 监听
    addfd(epfd, 0, false);
    if(epfd == -1)
    {
        perror("fail to create epoll!\n");
        exit(EXIT_FAILURE);
    }
       
    
    while(1)
    {
        int nfds = epoll_wait(epfd, events, MAX_EVENT_NUMBER, -1); //拿出就绪的文件描述符并进行处理
        if(nfds < 0)
        {
            printf("epoll failure!\n");
            break;
        }
        lt_process(events, nfds, epfd, sockfd);
    }
    //free(events);
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
