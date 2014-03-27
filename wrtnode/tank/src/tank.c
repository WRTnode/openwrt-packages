/*
 * Spi pin to control motor rotation (using spidev driver)
 * Copyright (C) 2014 WRTnode machine team.
 * This program is free software; you can redistribute it and/or modify
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/spidev32766.1";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay;

static void transfer(int fd,int argc,char *argv[])
{
	int ret;
	int num;
	int8_t tx[] = {
		0, 0, 0, 0
	};
	int8_t *rx = NULL;
	if(argc < 3){
		printf("error : Command line arguments to be greater than or equal to 3 \n");
		return ;
	}
	if(strcmp(argv[1] ,"l") == 0){
		tx[0] = 1;
		num = atoi(argv[2]);
		tx[3] = num;
		printf("The tank left wheel turn ,the speed is %d\n",num);
	}
	if(strcmp(argv[1] ,"r") == 0){
		tx[0] = 2;
		num = atoi(argv[2]);
		tx[3] = num;
		printf("The tank right wheel turn ,the speed is %d\n",num);
	}
	if(strcmp(argv[1] ,"m") == 0){
		tx[0] = 3;
		num = atoi(argv[2]);
		tx[2] = num;
		tx[3] = num;
		printf("The tank right wheel and left wheel turn at the same time ,the speed is %d\n",num);
	}
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

}

int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	transfer(fd,argc,argv);

	close(fd);

	return ret;
}
