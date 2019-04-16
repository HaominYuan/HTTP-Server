#ifndef __PRIORITY_QUEUE_H__
#define __PRIORITY_QUEUE_H__
#include <stdio.h>
#define PR_DEFAULT_SIZE 10


#define T PQueue_T
typedef struct T *T;
typedef int (*pq_comparator_pt)(void *pi, void *pj);

T pqueue_new(pq_comparator_pt comp, size_t capability);
int pqueue_is_empty(T self);
size_t pqueue_size(T self);
void *pqueue_min(T self);
int pqueue_delmin(T self);
int pqueue_insert(T self, void *item);
int pqueue_sink(T self, size_t i);
void pqueue_destroy(T *self);

#undef T
#endif
