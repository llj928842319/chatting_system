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
#include "color_output.h"
#include "scanbox.h"
#include "chat.h"


#define MAX 80


//function designed for chat between client and server
void func(int connfd)
{
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
	}
}

