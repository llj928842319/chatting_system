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
#include <sys/types.h>
#include <fcntl.h>

#include <sys/epoll.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include "color_output.h"
#include "scanbox.h"
#include "chat.h"





#define MAX_EVENT_NUMBER 1024   //事件总数量
#define BUFFER_SIZE 10          //缓冲区大小，这里为10个字节
#define ENABLE_ET 0             //ET模式


#define MAX 80


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
    struct epoll_event event;  //为当前fd设置事件
    event.data.fd = fd;        //指向当前fd
    event.events = EPOLLIN;    //使得fd可读
    if(enable_et)
    {
        event.events |= EPOLLET; //设置为边缘触发
    }
   // epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);  //将fd添加到内核中的epoll实例中
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1){
        perror("epoll_ctl wrong");
        exit(EXIT_FAILURE);
    }
    setnonblocking(fd);  //设为非阻塞模式                      
}


//function designed for chat between client and server
void func(int connfd)
{
    struct epoll_event events[MAX_EVENT_NUMBER];
    int nfds;
    int n = 0;
    char buff[MAX] = {0};

    boxout();
    int epoll_fd = epoll_create(5);  //在内核中创建epoll实例，flag为5只是为了分配空间用，实际可以不用带
    if(epoll_fd == -1)
    {
        perror("fail to create epoll!\n");
        exit(1);
    }

    addfd(epoll_fd, connfd, true); //添加文件描述符到epoll对象中      接受对方文件
    addfd(epoll_fd, 0, true); 
    for(;;){
        nfds = epoll_wait(epoll_fd, events, MAX_EVENT_NUMBER, -1);
        if(nfds == -1) {
               perror("epoll_wait");
               exit(EXIT_FAILURE);
            }

        for(n = 0; n<nfds; ++n){

            if(events[n].data.fd == connfd){
            //read the message from client and copy int in buffer
                
                bzero(buff, MAX);
                int ret = read(connfd, buff, sizeof(buff));
                if (ret == 0) {
                printf("Client exit\n");			
		        } 
                buff[ret-1] = '\0';
                newMessage("client", buff);		
                boxout();
            }            
            else if (events[n].data.fd == 0){
                boxout();
                bzero(buff, MAX);
                scantext(buff, MAX);
                write(connfd, buff, sizeof(buff));
            }
            else{
               

            }
            
    }
 }
}
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


