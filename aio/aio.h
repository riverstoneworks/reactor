/*
 * aio.h
 *
 *  Created on: Nov 1, 2018
 *      Author: Like.Z(sxpc722@aliyun.com)
 */

#ifndef AIO_H_
#define AIO_H_
#include <threads.h>
#include <linux/aio_abi.h>


typedef struct _io_res{
	__s64 res;
	__s64 res2;
	void* data;
	int(*cb)(void*);
}IORes;

struct _aio_srv;
struct aio_srv{
	unsigned long nr_events;
	struct _aio_srv *_aio;
	const thrd_t * thd_get_ioev;
};

extern int aio_srv_init(struct aio_srv *);

extern int aio_srv_destroy(struct aio_srv*);

extern long aio_submit(struct aio_srv* ,struct iocb* , long int );

#endif /* AIO_H_ */
