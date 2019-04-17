#include <stdio.h>
#include <stdio.h>
#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include "util.h"
#define MAX_BUF 8124

#define HTTP_PARSE_INVALID_METHOD  10
#define HTTP_PARSE_INVALID_REQUEST 11
#define HTTP_PARSE_INVALID_HEADER  12

#define HTTP_UNKNOWN 0x0001
#define HTTP_GET     0x0002
#define HTTP_HEAD    0x0003
#define HTTP_POST    0x0008

#define HTTP_OK           200
#define HTTP_NOT_MODIFIED 304
#define HTTP_NOT_FOUND    404

struct http_request_s {
	void *root;
	int fd;
	int epfd;
	void *timer;
	char buf[MAX_BUF];
	size_t pos, last;
	int state;
	void *request_start;
	void *method_end;
	int method;
	void *uri_start;
	void *uri_end;
	int http_major;
	int http_minor;
	void *request_end;
};

int init_request(struct http_request_s *r, int fd, int epfd, struct conf_s *cf);
int http_close_conn(struct http_request_s *r);


#endif
