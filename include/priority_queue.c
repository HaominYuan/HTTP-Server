#include "priority_queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define T PQueue_T
struct PQueue_T {
	void **pq;
	size_t size;
	size_t capability;
	pq_comparator_pt comp;
};


T pqueue_new(pq_comparator_pt comp, size_t capability) {
	T self = malloc(sizeof(*self));

	self->pq = malloc(sizeof(void *) * capability);
	if (!self->pq) {
		printf("pq_init: malloc failed\n");
		return NULL;
	}

	self->size = 0;
	self->capability = capability;
	self->comp = comp;
	return self;
}

void pqueue_destroy(T *self) {
	free((*self)->pq);
	free(*self);
	*self = NULL;
}

int pqueue_is_empty(T self) {
	return self->size == 0 ? 1 : 0;
}

size_t pqueue_size(T self) {
	return self->size;
}

void *pqueue_min(T self) {
	if (pqueue_is_empty(self)) return NULL;
	return self->pq[0];
}

static int recapability(T self, size_t new_capability) {
	if (new_capability <= self->size) {
		printf("recapability: new_capability is to small\n");
		return -1;
	}

	void **new_ptr = (void **) malloc(sizeof(void *) * new_capability);
	if (!new_ptr) {
		printf("recapability: malloc failed\n");
		return -1;
	}

	memcpy(new_ptr, self->pq, sizeof(void *) * (self->size));
	free(self->pq);
	self->pq = new_ptr;
	self->capability = new_capability;
	return 0;
}

static void exchange(T self, size_t i, size_t j) {
	void *tmp = self->pq[i];
	self->pq[i] = self->pq[j];
	self->pq[j] = tmp;
}


static void swim(T self, size_t k) {
	while (k > 0 && self->comp(self->pq[k], self->pq[(k - 1) / 2])) {
		exchange(self, k, (k - 1) / 2);
		k = (k - 1) / 2;
	}
}

static size_t sink(T self, size_t i) {
	size_t j, size = self->size;
	void *tmp = self->pq[i];
	for (j = 2 * i + 1; j < size; j = 2 * j + 1) {
		if (j + 1 < size && self->comp(self->pq[j + 1], self->pq[j])) j++;
		if (!self->comp(self->pq[j], tmp)) break;
		self->pq[i] = self->pq[j];
		i = j;
	}
	self->pq[i] = tmp;
	return i;
}

int pqueue_delmin(T self) {
	if (pqueue_is_empty(self)) {
		return -1;
	}

	exchange(self, 0, self->size - 1);
	self->size--;
	sink(self, 0);
	if (self->size <= self->capability / 4) {
		if (recapability(self, self->capability / 2) < 0) {
			return -1;
		}
	}
	return 0;
}

int pqueue_insert(T self, void *item) {
	if (self->size == self->capability) {
		if (recapability(self, self->capability * 2) < 0) {
			return -1;
		}
	}

	self->pq[self->size++] = item;
	swim(self, self->size - 1);
	return 0;
}

int pqueue_sink(T self, size_t i) {
	return sink(self, i);
}
