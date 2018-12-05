/*
 * type.h
 *
 *  Created on: Dec 5, 2018
 *      Author: Like.Z(sxpc722@aliyun.com)
 */

#ifndef REACTOR_TYPE_H_
#define REACTOR_TYPE_H_

#include <threads.h>

struct ready_queue{
	struct task** task;
	unsigned short head;
	unsigned short end;
	const unsigned short cap;
	int efd;			//read write signal
	thrd_t ex_thd;
	int lock_head;
	int *run_flag;
};

struct reactor{
	int(*dispatch)(struct reactor*,struct task*,int num);
	struct ready_queue* ready_queue;
	int run_flag;
	const unsigned short nq;
};

/*
 * Circular queue operation
 * The length of queue is 1 more then capability
 * when head==end, the queue is empty.
 */
#define queue_used(rq) ((rq->end-rq->head+rq->cap+1)%(rq->cap+1))

#define queue_left(rq) ((rq->head-rq->end+rq->cap)%(rq->cap+1))

static inline int queue_in(struct ready_queue* rq,struct task* t,int n){
	int i=0;
	for(int j;i<n&&((j=(rq->end+1)%(rq->cap+1))!=rq->head);++i){
			rq->task[rq->end]=t+i;
			rq->end=j;
	}
	eventfd_write(rq->efd,i);
	return i;
}

#endif /* REACTOR_TYPE_H_ */
