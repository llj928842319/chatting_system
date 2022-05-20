#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "color_output.h"
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "scanbox.h"
#include "chat.h"

#define MAX 80
#define PORT 8080
#define SA struct sockaddr


//drive function
int main()
{
	printf("\033c\n");

	int sockfd = 0, connfd = 0,  ret = 0;
	unsigned int len = 0;
	int enable = 1;
	struct sockaddr_in servaddr, cli;
	bzero(&servaddr, sizeof(servaddr));
	len = sizeof(cli);
	
	// assign ip,port
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("0.0.0.0"); // 0.0.0.0
	servaddr.sin_port = htons(PORT);


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


	ret = listen(sockfd, 5);
	if (ret != 0) {
		perror("listen failed...\n");
		exit(0);
	}
	printf("server listening...\n");


	connfd = accept(sockfd, (SA*)&cli, &len);
	if (connfd < 0) {
		perror("server accept failed..\n");
		exit(0);
	}
	printf("%d\n",connfd);
	printf("server accept the client...\n");
	
	//function for chatting between client and server
	func(connfd);
	
	//after chatting close the socket
	close(sockfd);
	return 0;
}
