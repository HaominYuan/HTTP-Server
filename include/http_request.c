#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "http_request.h"

int init_request(struct http_request_s *r, int fd, int epfd, struct conf_s *cf) {
	r->fd = fd;
	r->epfd = epfd;
	r->root = cf->root;
	r->pos = r->last = 0;
	r->state = 0;
	return 0;
}

int http_close_conn(struct http_request_s *r) {
	close(r->fd);
	free(r);
	return 0;
}
