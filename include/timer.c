#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include "timer.h"
#include "priority_queue.h"

#define PQ_DEFAULT_SIZE 10
#define TIMER_INFINITE -1

PQueue_T timer;
size_t current_msec;

static int timer_comp(void *p1, void *p2) {
	return ((timer_node *)p1)->key < ((timer_node *)p2)->key ? 1 : 0;
}

static void time_update() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	current_msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int timer_init() {
	timer = pqueue_new(timer_comp, PQ_DEFAULT_SIZE);
	time_update();
	return 0;
}

int find_timer() {
	timer_node *timer_node;
	int time = TIMER_INFINITE;

	while (!pqueue_is_empty(timer)) {
		time_update();
		timer_node = pqueue_min(timer);

		if (timer_node->deleted) {
			pqueue_delmin(timer);
			free(timer_node);
			continue;
		}

		time = (int) (timer_node->key - current_msec);
		time = time > 0 ? time : 0;
		break;
	}

	return time;
}

void handle_expire_timers() {
	timer_node *node;

	while (!pqueue_is_empty(timer)) {
		time_update();
		node = pqueue_min(timer);

		if (node->deleted) {
			pqueue_delmin(timer);
			free(node);
			continue;
		}

		if (node->key > current_msec) {
			return;
		}

		if (node->handler) {
			node->handler(node->rq);
		}

		pqueue_delmin(timer);
		free(node);
	}
}


void add_timer(struct http_request_s *rq, size_t timeout, timer_handler_pt handler) {
	timer_node *node = malloc(sizeof(timer_node));

	time_update();
	rq->timer = node;
	node->key = current_msec + timeout;
	node->deleted = 0;
	node->handler = handler;
	node->rq = rq;

	pqueue_insert(timer, node);
}

void del_timer(struct http_request_s *rp) {
	time_update();
	timer_node *node = rp->timer;
	node->deleted = 1;
}
