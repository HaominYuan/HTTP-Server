#ifndef __TIMER_H__
#define __TIMER_H__

#include "http_request.h"
#include "stdio.h"

typedef int (*timer_handler_pt)(struct http_request_s *rp);


typedef struct timer_node_s {
	size_t key;
	int deleted;
	timer_handler_pt handler;
	struct http_request_s *rq;
} timer_node;

int timer_init();
int find_timer();
void handle_expire_timers();

void add_timer(struct http_request_s *rq, size_t timeout, timer_handler_pt handler);
void del_timer(struct http_request_s *rq);

#endif
