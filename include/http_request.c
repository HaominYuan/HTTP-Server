#include "http_request.h"


int init_request(struct http_request_s *r, int fd, int epfd, struct conf_s *cf) {
	r->fd = fd;
	r->epfd = epfd;
	r->root = cf->root;

	return 0;
}
