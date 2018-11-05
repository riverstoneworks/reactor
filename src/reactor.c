
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
#include <sys/syscall.h>
#include <linux/aio_abi.h>
#include <threads.h>
#include <string.h>
#include <error.h>
#include <time.h>
#include "reactor/reactor.h"

int ret_res(struct io_event* ioev){
	((struct _io_res *)(ioev->data))->res=ioev->res;
	((struct _io_res*)(ioev->data))->res2=ioev->res2;
	struct iocb* io=(void*)(ioev->obj);
	io->aio_resfd=1;

	return 0;
}

struct _aio_srv{
	aio_context_t ctx;
	struct io_event* ioev;
	thrd_t thd;
};


int aio_srv_get_ioev(struct aio_srv* a){
	int n=0;
	while(1){
		n=syscall(SYS_io_getevents, a->_aio->ctx, 1, a->nr_events, a->_aio->ioev ,NULL);
		if(n<0){
			perror("io_getevents");
			return n;
		}
		while(n-->0){
			((struct _io_res *)(a->_aio->ioev[n].data))->cb(a->_aio->ioev+n);
		}
	}
	return 0;
}

int aio_srv_init(struct aio_srv * a){
	//new
	a->_aio=calloc(1,sizeof(struct _aio_srv));
	a->_aio->ioev=malloc(sizeof(struct io_event)*a->nr_events);

	int n=syscall(SYS_io_setup,a->nr_events,&(a->_aio->ctx));

	if(n<0)
		return n;
	else
		n=thrd_create(&(a->_aio->thd), (int(*)(void*))aio_srv_get_ioev, a);

	return n;
}

int aio_srv_destroy(struct aio_srv* a){

	int n,r;
	if(0>(n=syscall(SYS_io_destroy,a->_aio->ctx)))
		return n;
	else if(0>(n=thrd_join(a->_aio->thd,&r)))
		return n;

	free(a->_aio->ioev);
	free(a->_aio);

	return n;
}

long aio_submit(struct aio_srv* as,struct iocb* io, long int nr){
	return syscall(SYS_io_submit,as->_aio->ctx,nr,&io);
}
