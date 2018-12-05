/*
 ============================================================================
 Name        : test.c
 Author      : Like.Z
 Version     :
 Copyright   : 2018 Like.Z
 Description : test in C
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <error.h>
#include <string.h>

#include "aio/aio.h"
#include "reactor/reactor.h"
#include "reactor/dispatcher.h"


int simpleTask(struct task* ts){
	struct{
		struct aio_srv* as;
	}*input=ts->data;
	struct{
		long long x;
		char buf[2049];
		struct iocb ios;
		struct aio_srv* as;
		struct _io_res iores;
	}*p=ts->data;
	struct reactor* r=ts->r;

	switch(ts->stat){
		case INIT:
			ts=malloc(sizeof(struct task));
			p=malloc(sizeof(*p));
			p->as=input->as;
			ts->data=p;
			ts->fun=simpleTask;
			ts->r=r;
			ts->stat=1;
			ready(ts);
			return 0;
		case 1:
			p->ios.aio_fildes = open("/home/xpc/Documents/Bookmarks", O_RDONLY);
			p->iores.data=ts;
			p->iores.cb=(int(*)(void*))ready;
			p->ios.aio_data = (__u64 ) &(p->iores);
			p->ios.aio_buf = (__u64 ) p->buf;
			p->ios.aio_nbytes = 64;
			p->ios.aio_lio_opcode = IOCB_CMD_PREAD;
			p->ios.aio_offset=0;
		case 2:
			p->ios.aio_offset += p->iores.res;
//			printf("\n%ld\n",p->ios.aio_offset);
			ts->stat=3;
			aio_submit(p->as,&(p->ios),1)<0?perror("aio_submit"):0;
			return 0;
		case 3:
			if(((IORes*)(p->ios.aio_data))->res2==0&&((IORes*)(p->ios.aio_data))->res>0){
				((char*)(p->ios.aio_buf))[p->iores.res]='\0';
				printf("%s",(char*)(p->ios.aio_buf));
				ts->stat=2;
				if(p->ios.aio_nbytes==((IORes*)(p->ios.aio_data))->res)
					ready(ts);
				return 0;
			}else
				printf("%lld : %lld\n",((IORes*)(p->ios.aio_data))->res,((IORes*)(p->ios.aio_data))->res2);
		default:
			close(p->ios.aio_fildes);
			break;
	}

	free(ts->data);
	free(ts);
	return 0;
}

int main(void) {

	struct aio_srv as={.nr_events=10};
	aio_srv_init(&as)<0?perror("e1"):0;

	struct reactor* rct=create_reactor(3,50,dispatch_by_left);

	struct{
		struct aio_srv* as;
	}input={&as};
	struct task t={
			.fun=simpleTask,
			.stat=INIT,
			.r=rct,
			.data=&input
	};
	simpleTask(&t);


	sleep(10);


	destory_reactor(rct);

	aio_srv_destroy(&as)<0?perror("aio_srv_destroy"):0;

	return EXIT_SUCCESS;
}
