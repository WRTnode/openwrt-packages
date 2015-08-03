/*
FileName    :spi_mt7688.h
Description :head file of spi bus on mt7688.
Author      :SchumyHao
Email       :schumy.haojl@gmail.com
Version     :V01
Date        :2015.08.02
*/

#ifndef __SPI_MT7688_H__
#define __SPI_MT7688_H__
/*
    include files
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "spi.h"

/* spi mt7688 port error number */
typedef enum {
    SPI_MT7688_ERR_OPEN = (int)SPI_ERR_LAST,
    SPI_MT7688_ERR_PARA,
    SPI_MT7688_ERR_FD,
    SPI_MT7688_ERR_SETTING,
    SPI_MT7688_ERR_READ,
    SPI_MT7688_ERR_WRITE,
    SPI_MT7688_ERR_DRAIN,

    SPI_MT7688_ERR_LAST
} spi_mt7688_err_t;

/*
    struct of posix serial port
*/
struct spi_mt7688 {
    //about spi mt7688 variables
    int fd;
    //spi base class
    struct spi_base* sb;

    //member functions
    int (*open)(struct spi_mt7688* s);
    int (*config_port)(struct spi_mt7688* s);
    int (*close)(struct spi_mt7688* s);
    int (*read)(struct spi_mt7688* s, char* dst, int size);
    int (*write)(struct spi_mt7688* s, const char* src, int size);
};

typedef struct {
    spi_init_t sp;
} spi_mt7688_init_t;

/*
    global functions
*/
struct spi_mt7688* spi_mt7688_port_init(spi_mt7688_init_t* psp);
#endif
