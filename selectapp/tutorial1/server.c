
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


#define BUFFER_SIZE 80
#define PORT 8080
#define SA struct sockaddr



//定义结构体
typedef struct newmessager{
    char name[32];//客户端姓名 也就是IP
    uint16_t port_name;//端口号
    int cfd;//对应的套接字
    //char type;//消息类型
    
    char text[BUFFER_SIZE];//消息内容，也就是缓存区 
    //in_port_t sin_port;			/* Port number.  */
    //struct in_addr sin_addr;		/* Internet address.  */
}NMSG;
NMSG nmsg;//实例化


//定义链表，存储客户端，
typedef struct client_list
{
    char name[32];//客户端名字
    int16_t portid;//端口
    int cfd;//套接字
    struct client_list *next;
}linklist, *linkList;

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
    strcpy(p->name, nmsg.name);//复制字符
    p->cfd = connfd;
    p->next = Head->next;
    Head->next = p;
}


int name_exist(linkList H ,NMSG nmsg,int connfd)
{
    linkList s = H->next;
    while (s){//如果服务器中已经有这个人了，就不存,并给该客户端发送消息
        if (strcmp(s->name, nmsg.name) == 0 && (!(*(s->portid) == nmsg.port_name) )){
            //free(s);
            return 0;
        }
        s=s->next;
    }
    //free(s);
    return 1;
}

//drive function
int main()
{
	printf("\033c\n");

	int sockfd = 0, ret = 0;
	int connfd = 0;
	int enable = 1;
	linkList Head;//头头
	linkList head_init();
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	
	// assign ip,port
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("0.0.0.0"); // 0.0.0.0
	servaddr.sin_port = htons(PORT);
    //client
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
	char name[30];
    int portname;


	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

	if (sockfd == -1) {
		perror("socket creation failed...\n");
		exit(0);
	}
	printf("socket successfully created...\n");


	ret = bind(sockfd, (SA*)&servaddr, sizeof(servaddr));
	if (ret != 0) {
		perror("socket bind failed...\n");
		exit(0);
	}
	printf("socket successfully binded..\n");


	int listen_fd = listen(sockfd, 5);
	if (listen_fd != 0) {
		perror("listen failed...\n");
		exit(0);
	}
	printf("server listening...\n");


	while(1){
		//chatting between client and server
	
    connfd = accept(ret, (struct sockaddr*)&client_address,  &client_addrlength);


	if(connfd == listen_fd){  //如果监听类型的描述符，则代表有新的client接入，则将其添加到内核中的epoll结构中
        
            
            int connfd = accept(listen_fd, (struct sockaddr*)&client_address, &client_addrlength); //创建连接并返回文件描述符（实际进行的三次握手过程）
            //char clientname[30] ;
            //long int l = 0;
            getpeername(connfd, (struct sockaddr*)&client_address, &client_addrlength);
            strcpy(nmsg.name, inet_ntoa(client_address.sin_addr));// 获取ip
            nmsg.port_name = client_address.sin_port;//获取端口
            printf("\nip:%s  port:%d 进来了 \n", nmsg.name, nmsg.port_name);
			//addfd(epoll_fd, connfd, false);  //添加到epoll结构中并初始化为LT模式
            // TODO: 把 connfd 加到自己的数据结构里
            if (name_exist(Head, nmsg, connfd)==0){//判断在链表结构里面
                bzero(nmsg.text, sizeof(nmsg.text));
                sprintf(nmsg.text,"The user ip:%s port:%d has been registered!\n",nmsg.name,nmsg.port_name);
                if (send(connfd, &nmsg.text, sizeof(nmsg.text), 0) == -1){
                    perror("[send_4]");
                    
                }
                bzero(nmsg.text, sizeof(nmsg.text));
                
            } 
            else{
                insert_client(Head, nmsg, connfd);//，如果没在链表结构里面，将这个客户端添加到我们的链表中
                
                linkList q = Head->next;      //新建一个暂时的链表用来遍历
                sprintf(nmsg.text, "welcome to join us ip:%s Port:%d!\n",nmsg.name,nmsg.port_name);
                while (q){
                    if (send(q->cfd, &nmsg.text, sizeof(nmsg.text), 0) == -1){
                        perror("[send_1]");
                    }
                    q = q->next;                   
                }
                //free(q);
                bzero(nmsg.text,sizeof(nmsg.text));
                

            }
	}
	else {
		int tuichu = 0;
		tuichu = read(connfd, nmsg.text, sizeof(nmsg.text));
		//after chatting close the socket
		if(tuichu = 0){
		//客户端退出
		close(connfd);
		}  

		if (connfd == EPOLLIN){
			printf("IP%s port %d说: %s", nmsg.name, nmsg. port_name, nmsg.text);
			
			//接受消息
		}

		
	}
	return 0;	
}
}



