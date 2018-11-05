/*
 * reactor.h
 *
 *  Created on: Nov 1, 2018
 *      Author: Like.Z(sxpc722@aliyun.com)
 */

#ifndef REACTOR_H_
#define REACTOR_H_
#include <linux/aio_abi.h>


typedef struct _io_res{
	__s64 res;
	__s64 res2;
	int(*cb)(struct io_event*);
}IORes;

struct _aio_srv;
struct aio_srv{
	long nr_events;
	struct _aio_srv* _aio;
};

extern int aio_srv_init(struct aio_srv *);

extern int aio_srv_destroy(struct aio_srv*);

extern long aio_submit(struct aio_srv* ,struct iocb* , long int );

extern int ret_res(struct io_event*);

#endif /* REACTOR_H_ */
