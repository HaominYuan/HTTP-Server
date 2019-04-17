#ifndef __HTTP_H__
#define __HTTP_H__

typedef struct mime_type_s {
	const char *type;
	const char *value;
} mime_type_t;

void do_request(void *infd);

#endif
