/*
 * reactor.h
 *
 *  Created on: Nov 1, 2018
 *      Author: Like.Z(sxpc722@aliyun.com)
 */

#ifndef REACTOR_H_
#define REACTOR_H_

struct reactor;

struct task{
	int (*fun)(struct task*); //function to be executed
	void* data;
	enum {
			CANCEL=-2,
			COMPLETE=-1,
			INIT=0
	} stat;	//status ID
	struct reactor* r;
};

extern struct reactor* create_reactor(int nq,int cap,int(*dispach)(struct reactor*,struct task*,int));
extern int task_cancel(int task_no,struct task*, struct reactor*);
extern int destory_reactor(struct reactor*);

extern int ready(struct task* );

extern int dispatch_by_left(struct reactor* r,struct task* t,int num);

#endif /* REACTOR_H_ */
