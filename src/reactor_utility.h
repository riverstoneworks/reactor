/*
 * type.h
 *
 *  Created on: Dec 5, 2018
 *      Author: Like.Z(sxpc722@aliyun.com)
 */

#ifndef REACTOR_UTILITY_H_
#define REACTOR_UTILITY_H_
#include <threads.h>
#include <stdatomic.h>
#include <stdbool.h>
#include "reactor/reactor.h"

struct ready_queue{
	struct task** task;
	thrd_t ex_thd;
	int efd;			//read write signal
	const unsigned short cap;
	volatile unsigned short head;
	volatile atomic_ushort end; // @suppress("Type cannot be resolved")
	volatile bool *run_flag;
};

struct reactor{
	struct ready_queue* ready_queue;
	int(*dispatch)(struct reactor*,struct task*,int num);
	const unsigned short nq;
	volatile bool run_flag;
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
	unsigned short j,ce;
	while(i<n){
		if((j=((ce=rq->end)+1)%(rq->cap+1))!=rq->head){
			if(atomic_compare_exchange_strong_explicit(&(rq->end),&ce,j,memory_order_relaxed,memory_order_relaxed))
				rq->task[ce]=t+i;
			else
				continue;
			++i;
		}else
			break;
	}

	return i>0?(eventfd_write(rq->efd,i)?-1:i):0;
}



#endif /* REACTOR_UTILITY_H_ */
