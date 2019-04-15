#include "util.h"
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdio.h>

#define LISTENQ 1024

int open_listenfd(int port) {
	int listenfd, optval = 1;
	struct sockaddr_in serveraddr;
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)  
		return -1; 
	/* 直接重用端口 */
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int)) < 0)
		return -1; 
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(port);
	if (bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
		return -1; 
	if (listen(listenfd, LISTENQ) < 0)
		return -1; 
	return listenfd;
}

int make_socket_non_blocking(int fd) {
	int flags, s;
	flags = fcntl(fd, F_GETFL, 0); 
	if (flags == -1) {
		perror("fnctl");
		return -1; 
	}   
	flags |= O_NONBLOCK;
	s = fcntl(fd, F_SETFL, flags);
	if (s == -1) {
		perror("fcntl");
		return -1; 
	}   
	return 0;
}
