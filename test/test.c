
/*
 ============================================================================
 Name        : test.c
 Author      : Like.Z
 Version     :
 Copyright   : 2018 Like.Z
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <error.h>
#include <string.h>

#include "reactor/reactor.h"


int main(void) {

	struct aio_srv as={.nr_events=10};
	aio_srv_init(&as)<0?perror("e1"):0;

	char * buf=malloc(1024);

	struct iocb ios={
					.aio_fildes=open("/home/xpc/Documents/Bookmarks",O_RDONLY),
					.aio_data=(__u64)(&(struct _io_res){0,0,ret_res}),
					.aio_buf=(__u64)buf,
					.aio_nbytes=64,
					.aio_lio_opcode=IOCB_CMD_PREAD,
					.aio_offset=0,
					.aio_resfd=0
	};

	aio_submit(&as,&ios,1)<0?perror("aio_submit"):0;

	sleep(1);

	if(ios.aio_resfd){
		printf("%s\n%lld:%lld\n",(char*)ios.aio_buf,((IORes*)ios.aio_data)->res,((IORes*)ios.aio_data)->res2);
	}

	close(ios.aio_fildes);

	sleep(1);
	aio_srv_destroy(&as)<0?perror("aio_srv_destroy"):0;
	return EXIT_SUCCESS;
}
