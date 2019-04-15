#ifndef __UTIL_H__
#define __UTIL_H__

int open_listenfd(int port);
int make_socket_non_blocking(int fd);

struct conf_s {
	void *root;
	int port;
	int thread_num;
};

#endif
