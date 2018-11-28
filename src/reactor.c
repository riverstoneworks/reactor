/*
 * reactor.c
 *
 *  Created on: Nov 4, 2018
 *      Author: Like.Z(sxpc722@aliyun.com)
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <threads.h>
#include <string.h>
#include <error.h>
#include <time.h>
#include <sys/syscall.h>
#include <sys/eventfd.h>

#include "reactor/reactor.h"

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
	int(*dispatch)(struct reactor*,struct task*);
	struct ready_queue* ready_queue;
	int run_flag;
	const unsigned short nq;
};

int task_exec(struct ready_queue* rq){
	eventfd_t i=0;
	struct task* t;
	while(*(rq->run_flag)){
		if(eventfd_read(rq->efd,&i)<0)
			return -1;
		for(int j=0;j<i;++j){
			t=rq->task[rq->head++];
			t->fun(t);
		}
	}

	return 0;
}

struct reactor* create_reactor(int nq,int cap,int(*dispatch)(struct reactor*,struct task*)){
	struct reactor* r=malloc(sizeof(struct reactor));
	if(!r)
		return NULL;
	*(unsigned short*)(&(r->nq))=nq;
	r->dispatch=dispatch;
	r->run_flag=1;

	if(!(r->ready_queue=malloc(sizeof(struct ready_queue)*nq)))
		return NULL;
	while(nq--){
		struct ready_queue* rq=r->ready_queue+nq;
		*(unsigned short*)&(rq->cap)=cap;
		rq->task=malloc(sizeof(struct task*)*(cap+1));
		rq->head=0;
		rq->end=0;
		rq->run_flag=&(r->run_flag);
		rq->efd=eventfd(0, 0);
		rq->lock_head=eventfd(1, EFD_SEMAPHORE);
		int eno=thrd_create(&(rq->ex_thd), (int(*)(void*))task_exec,rq );
		if(eno<0||rq->efd<0||rq->task==NULL){
			destory_reactor(r);
			return NULL;
		}
	}
	return r;
}

//do nothing
int fn(struct task* v){return 0;}

int destory_reactor(struct reactor* r){
	int i=r->nq,rt;
	r->run_flag=0;
	struct task tt={
			.r=r,
			.fun=fn
	};
	while(i--){
		struct ready_queue* rq=r->ready_queue+i;
		if(rq->end==rq->head){
			int tmp=-1;
			eventfd_read(rq->lock_head,NULL);
			if(rq->cap-((rq->end-rq->head+rq->cap+1)%(rq->cap+1))>0)
				tmp=rq->end=(rq->end+1)%(rq->cap+1);
			eventfd_write(rq->lock_head,1);

			if(tmp>0){
				rq->task[--tmp]=&tt;
				eventfd_write(rq->efd,1);
			}
		}
		thrd_join(rq->ex_thd,&rt);
		free(rq->task);
		close(rq->efd);
	}
	free(r->ready_queue);
	free(r);

	return 0;
}

//submit task to ready queue
int ready(struct task* ts){
	return (ts->r->dispatch)(ts->r,ts);
}


int dispatch_by_left(struct reactor* r,struct task* t){
	int n=r->nq,i=0,left=0,tmp;
	struct ready_queue* rq;
	while(n--){
		rq=r->ready_queue+n;
		tmp=rq->cap-((rq->end-rq->head+rq->cap+1)%(rq->cap+1));
		if(left<tmp){
			left=tmp;
			i=n;
		}
	}
	tmp=-1;
	if(left>0){
		rq=r->ready_queue+i;
		eventfd_read(rq->lock_head,NULL);
		if(rq->cap-((rq->end-rq->head+rq->cap+1)%(rq->cap+1))>0)
			tmp=rq->end=(rq->end+1)%(rq->cap+1);
		eventfd_write(rq->lock_head,1);

		if(tmp>0){
			rq->task[--tmp]=t;
			eventfd_write(rq->efd,1);
		}

	}

	return tmp<0?-1:rq->cap*i+tmp;
}


