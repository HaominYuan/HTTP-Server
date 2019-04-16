#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include "util.h"

struct http_request_s {
	void *root;
	int fd;
	int epfd;
	void *timer;
};

int init_request(struct http_request_s *r, int fd, int epfd, struct conf_s *cf);
int http_close_conn(struct http_request_s *r);


#endif
