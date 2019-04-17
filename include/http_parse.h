#ifndef __HTTP_PARSE_H__
#define __HTTP_PARSE_H__

#include <stdio.h>
#include "http_request.h"

#define CR '\r'
#define LF '\n'

int http_parse_request_line(struct http_request_s *r);
int http_parse_request_body(struct http_request_s *r);

#endif
