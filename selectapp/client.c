#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "color_output.h"
#include "scanbox.h"
#include "chat.h"
#define MAX 80
#define PORT 8080
#define SA struct sockaddr



int main()
{
	printf("\033c\n");

	int sockfd = 0;
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));

	// assign ip,port
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);
	
	//socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1 ) {
		printf("socket creation failed...\n");
		exit(0);
	}
	printf("socket successfully created...\n");

	
	//connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0){
		perror("connection with the server failed...\n");
		exit(0);
	}
	printf("connect the client...\n");

	//function for chatting between client and server
	func(sockfd);


	//after chatting close the socket
	close(sockfd);
	return 0;
}
