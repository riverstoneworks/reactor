/*
 * reactor.h
 *
 *  Created on: Nov 1, 2018
 *      Author: Like.Z(sxpc722@aliyun.com)
 */

#ifndef REACTOR_H_
#define REACTOR_H_
#include <threads.h>
struct reactor;

typedef struct{
	struct reactor* rct;
	const thrd_t ** rq_thd;
	unsigned short rq_thd_num;
}Reactor;

struct task{
	int (*fun)(struct task*); //function to be executed
	void* data;
	enum {
			CANCEL=-2,
			COMPLETE=-1,
			INIT=0
	} stat;	//status ID
	Reactor* r;
};

extern Reactor* create_reactor(const unsigned short nq,const unsigned short cap,int(*dispach)(struct reactor*,struct task*,int));
extern int task_cancel(int task_no,struct task*, Reactor*);
extern int destory_reactor(Reactor*);

extern int ready(struct task* );

#endif /* REACTOR_H_ */
