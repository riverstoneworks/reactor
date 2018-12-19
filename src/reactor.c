/*
 * reactor.c
 *
 *  Created on: Nov 4, 2018
 *      Author: Like.Z(sxpc722@aliyun.com)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <sys/syscall.h>
#include <sys/eventfd.h>
#include <stdatomic.h>
#include <stdbool.h>
#include "reactor/reactor.h"
#include "reactor_utility.h"

/*
 * task scheduler
 */
static int scheduler(struct ready_queue* rq){
	sleep(5);
	eventfd_t i=0;
	struct task* t;
	while(*(rq->run_flag)){
		if(eventfd_read(rq->efd,&i))
			return -1;
		else
			for(int j=0;j<i;++j){
				t=rq->task[rq->head];
				rq->head=(rq->head+1)%(rq->cap+1);
				t->fun(t);
			}
	}

	return 0;
}

Reactor* create_reactor(const unsigned short nq,const unsigned short cap,int(*dispatch)(struct reactor*,struct task*,int)){
	Reactor* rt=malloc(sizeof(Reactor));
	rt->rq_thd_num=nq;
	(rt->rq_thd)=malloc(sizeof(thrd_t*)*nq);

	struct reactor *r=rt->rct=malloc(sizeof(struct reactor));
	if(!r)
		return NULL;
	*(unsigned short*)(&(r->nq))=nq;
	r->dispatch=dispatch;
	r->run_flag=true;

	if(!(r->ready_queue=malloc(sizeof(struct ready_queue)*nq)))
		return NULL;
	for(int i=0;i<nq;++i){
		struct ready_queue* rq=r->ready_queue+i;
		*(unsigned short*)&(rq->cap)=cap;
//		*(unsigned short*)&(rq->rq_id)=i;
		rq->task=malloc(sizeof(struct task*)*(cap+1));
		rq->head=0;
		rq->end=0;
		rq->run_flag=&(r->run_flag);
		rq->efd=eventfd(0, 0);
		int eno=thrd_create(&(rq->ex_thd), (int(*)(void*))scheduler,rq );
		if(eno<0||rq->efd<0||rq->task==NULL){
			perror("reactor init error!");
			destory_reactor(rt);
			return NULL;
		}
		rt->rq_thd[i]=&(rq->ex_thd);
	}
	return rt;
}

//do nothing
static int fn(struct task* v){return 0;}

int destory_reactor(Reactor* rct){
	struct reactor* r=rct->rct;
	r->run_flag=false;
	int i=r->nq,rt;
	struct task tt={
			.r=rct,
			.fun=fn
	};
	while(i--){
		struct ready_queue* rq=r->ready_queue+i;
		if(rq->end==rq->head){
			queue_in(rq,&tt,1);
		}
		thrd_join(rq->ex_thd,&rt);
		free(rq->task);
		close(rq->efd);
	}
	free(r->ready_queue);
	free(rct->rct);
	free(rct->rq_thd);
	free(rct);

	puts("reactor destroyed!");
	return 0;
}

//submit task to ready queue
int ready(struct task* ts){
//	printf("%d task(s) dispatched!\n",(ts->r->dispatch)(ts->r,ts,1));
	return (ts->r->rct->dispatch)(ts->r->rct,ts,1);
}
