/*
 * dispatcher.c
 *
 *  Created on: Dec 5, 2018
 *      Author: Like.Z(sxpc722@aliyun.com)
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include "reactor/reactor.h"
#include "reactor/dispatcher.h"
#include "reactor_type.h"


int dispatch_by_left(struct reactor* r,struct task* t,int task_num){
	int n=r->nq,i=0,left=0,tmp;
	struct ready_queue* rq;
	while(n--){
		rq=r->ready_queue+n;
		tmp=queue_left(rq);
		if(left<tmp){
			left=tmp;
			i=n;
		}
	}

	if(left>0){
		rq=r->ready_queue+i;
		eventfd_read(rq->lock_head,NULL);
		n=queue_in(rq,t,task_num);
		eventfd_write(rq->lock_head,1);
//		printf("\nh: %d: e: %d\n",rq->head,rq->end);
	}

	return n;
}
