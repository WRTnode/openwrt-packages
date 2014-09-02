/*
 * flash MAC  
 * Copyright (C) 2014 WRTnode machine team.
 * This program is free software; you can redistribute it and/or modify
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdio.h>             
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#define MEMERASE		_IOW('M', 2, struct erase_info_user)

struct erase_info_user {
		uint32_t start;
		uint32_t length;
};

int main(int argc,char *argv[])
{
	int fd,sz,i,offset,offset0,offset1,offset2;
	int mac[6];
	sz = 0x10000;
	offset0 = 0x04;
	offset1 = 0x28;
	offset2 = 0x2E;
	unsigned char *buf;
	buf = (unsigned char *)malloc(sz);
	for(i=0;i<6;i++){
		mac[i] = strtol(argv[i+1], NULL, 16);
	}
	if(mac[0]%4 !=0)
	{
		printf("ple input a effective MAC\n");
		return -1;
	}
	fd = open("/dev/mtd2", O_RDWR | O_SYNC);
	if (fd < 0) {
		fprintf(stderr, "failed to open mtd2\n");
		free(buf);
		return -1;
	}
  	//read-----------
  	lseek(fd, 0, SEEK_SET);
	//backup
	if (read(fd, buf, sz) != sz) {
		fprintf(stderr, "failed to read %d bytes from mtd2\n",
				sz);
		free(buf);
		close(fd);
		return -1;
	}
	//erase
	struct erase_info_user ei;
	lseek(fd, 0, SEEK_SET);
	ei.start = 0;
	ei.length = sz;
	if (ioctl(fd, MEMERASE, &ei) < 0) {
		fprintf(stderr, "failed to erase mtd2\n");
		free(buf);
		close(fd);
		return -1;
	}
	for(i=0;i<3;i++){
		if(i==0)
			offset=offset0;
		if(i==1){
			offset=offset1;
			mac[5] +=1;
		}
		if(i==2){
			offset=offset2;
			mac[5] -=1;
		}
		for(i=0;i<6;i++){
			*(buf + (offset) + i) = mac[i];
		}
	}
	//write
	lseek(fd, 0, SEEK_SET);
 	if (write(fd, buf, sz) == -1) {
		fprintf(stderr, "failed to write mtd%d\n", i);
		free(buf);
		close(fd);
		return -1;
	}
	free(buf);
	close(fd);
	return 0;
}
