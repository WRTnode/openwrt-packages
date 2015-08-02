/*
FileName    :spi_mt7688.c
Description :Source code of spi bus on mt7688 port.
Author      :SchumyHao
Email       :schumy.haojl@gmail.com
Version     :V01
Date        :2015.08.02
*/
/*
    include files
*/
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>

#include "spi.h"
#include "spi_mt7688.h"

#define SPI_MCU_READ           (0x01)
#define SPI_MCU_READ_LEN       (0x04)
#define SPI_MCU_WRITE          (0x10)
#define SPI_MCU_WRITE_LEN      (0x40)
#define SPI_MCU_READ_STATUS    (0xff)
#define SPI_STATUS_7688_READ_FROM_STM32_E    (1<<0)
#define SPI_STATUS_7688_READ_FROM_STM32_NE   (0<<0)
#define SPI_STATUS_7688_WRITE_TO_STM32_F     (1<<1)
#define SPI_STATUS_7688_WRITE_TO_STM32_NF    (0<<1)
#define SPI_STATUS_OK                        (0x80)

#define SPI_MCU_READ_DELAY_US   (200)
#define SPI_MCU_WRITE_DELAY_US  (200)
#define SPI_MCU_CHECK_STATUS_DELAY_US   (100000)

static inline unsigned char read_status(int fd)
{
    unsigned char ch = 0;
    ioctl(fd, SPI_MCU_READ_STATUS, &ch);
    usleep(SPI_MCU_READ_DELAY_US);
    ioctl(fd, SPI_MCU_READ_STATUS, &ch);
    usleep(SPI_MCU_READ_DELAY_US);

    return ch;
}

static inline unsigned char read_len(int fd)
{
    unsigned char ch = 0;
    ioctl(fd, SPI_MCU_READ_LEN, &ch);
    usleep(SPI_MCU_READ_DELAY_US);
    ioctl(fd, SPI_MCU_READ_LEN, &ch);
    usleep(SPI_MCU_READ_DELAY_US);

    return ch;
}

static inline unsigned char read_ch(int fd)
{
    unsigned char ch = 0;
    ioctl(fd, SPI_MCU_READ, &ch);
    usleep(SPI_MCU_READ_DELAY_US);
    ioctl(fd, SPI_MCU_READ, &ch);
    usleep(SPI_MCU_READ_DELAY_US);

    return ch;
}

static inline void put_ch(int fd, unsigned char ch)
{
    ioctl(fd, SPI_MCU_WRITE, &ch);
    usleep(SPI_MCU_WRITE_DELAY_US);
}

static inline void put_len(int fd, unsigned char len)
{
    ioctl(fd, SPI_MCU_WRITE_LEN, &len);
    usleep(SPI_MCU_WRITE_DELAY_US);
}

/*
    member functions
*/
static int spi_mt7688_open(struct spi_mt7688* s)
{
    int ret = 0;
    if(NULL == s) {
        ret = -SPI_ERR_INPUT_NULL;
        goto SPI_MT7688_OPEN_INPUT_ERROR;
    }
    //can not open posix serial port twice.
    if(s->sb->is_open(s->sb)) {
        printf("open fail2\n");
        ret = -SPI_MT7688_ERR_OPEN;
        goto SPI_MT7688_OPEN_REOPEN_ERROR;
    }

    //open spi mt7688 port
    s->fd = open(s->sb->get_port(s->sb), O_RDWR);
    if(s->fd < 0) {
        printf("open fail\n");
        ret = -SPI_MT7688_ERR_OPEN;
        goto SPI_MT7688_OPEN_OPEN_ERROR;
    }
    //set is_open flag.
    s->sb->set_is_open(s->sb);
    //config spi mt7688 port. close it if config failed.
    if((ret = s->config_port(s)) < 0) {
        goto SPI_MT7688_OPEN_CONFIG_ERROR;
    }
    return 0;

SPI_MT7688_OPEN_CONFIG_ERROR:
    s->sb->clean_is_open(s->sb);
    close(s->fd);
SPI_MT7688_OPEN_OPEN_ERROR:
    s->fd = 0;
SPI_MT7688_OPEN_REOPEN_ERROR:
SPI_MT7688_OPEN_INPUT_ERROR:
    printf("open port fail\n");
    return ret;
}

static int spi_mt7688_config_port(struct spi_mt7688* s)
{
    int ret = 0;

    if(NULL == s) {
        ret = -SPI_ERR_INPUT_NULL;
        goto SPI_MT7688_CONFIG_PORT_INPUT_ERROR;
    }

    if(s->fd <= 0) {
        ret = -SPI_MT7688_ERR_FD;
        goto SPI_MT7688_CONFIG_PORT_FD_ERROR;
    }
    return 0;

SPI_MT7688_CONFIG_PORT_FD_ERROR:
SPI_MT7688_CONFIG_PORT_INPUT_ERROR:
    return ret;
}

static int spi_mt7688_close(struct spi_mt7688* s)
{
    int ret = 0;

    if(NULL == s) {
        ret = -SPI_ERR_INPUT_NULL;
        goto SPI_MT7688_CLOSE_INPUT_ERROR;
    }

    if(s->sb->is_open(s->sb)) {
        /* clean serial open flag */
        s->sb->clean_is_open(s->sb);
        /* closd fd and set fd to 0 */
        close(s->fd);
        s->fd = 0;
        /* free port name */
        s->sb->clean_port(s->sb);
        /* free spi_base */
        free(s->sb);
        /* free spi mt7688 */
        free(s);
    }
    return 0;

SPI_MT7688_CLOSE_INPUT_ERROR:
    return ret;
}

static int spi_mt7688_read(struct spi_mt7688* s, char* dst, int size)
{
    int ret = 0;
    int nleft = size;
    char* ptr = dst;
    int readn = 0;

    if((NULL==s)||(NULL==dst)) {
        ret = -SPI_ERR_INPUT_NULL;
        goto SPI_MT7688_READ_INPUT_ERROR;
    }

    if(!(s->sb->is_open(s->sb))) {
        ret = -SPI_MT7688_ERR_OPEN;
        goto SPI_MT7688_READ_PORT_NOT_OPEN_ERROR;
    }

    while (nleft > 0) {
        long busy_wait_us = 0;
        unsigned char status = SPI_STATUS_OK;
        int i = 0;
        busy_wait_us = 1000*(s->sb->timeout.sec) + (s->sb->timeout.usec)/1000;
        do {
            status = read_status(s->fd);
            if((status & SPI_STATUS_OK) &&
               (!(status & SPI_STATUS_7688_READ_FROM_STM32_E))) {
                break;
            }
            usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
        } while((0 == busy_wait_us)||((busy_wait_us-SPI_MCU_CHECK_STATUS_DELAY_US) > 0));
        readn = read_len(s->fd);
        for(i=0; i<readn; i++) {
            ptr[i] = read_ch(s->fd);
        }
        nleft -= readn;
        ptr += readn;
    }
    return (size-nleft);

SPI_MT7688_READ_PORT_NOT_OPEN_ERROR:
SPI_MT7688_READ_INPUT_ERROR:
    return ret;
}

static int spi_mt7688_write(struct spi_mt7688* s, const char* src, int size)
{
    int ret = 0;
    int nleft = size;
    int writen = 0;
    const char* ptr = src;

    if((NULL==s)||(NULL==src)) {
        ret = -SPI_ERR_INPUT_NULL;
        goto SPI_MT7688_WRITE_INPUT_ERROR;
    }
    //check whether the port is open or not
    if(!(s->sb->is_open(s->sb))) {
        ret = -SPI_MT7688_ERR_OPEN;
        goto SPI_MT7688_WRITE_PORT_NOT_OPEN_ERROR;
    }
    //write until nleft is empty
    while(nleft > 0) {
        long busy_wait_us = 0;
        unsigned char status = SPI_STATUS_OK;
        int i = 0;
        busy_wait_us = 1000*(s->sb->timeout.sec) + (s->sb->timeout.usec)/1000;
        do {
            status = read_status(s->fd);
            if((status & SPI_STATUS_OK) &&
               (!(status & SPI_STATUS_7688_WRITE_TO_STM32_F))) {
                break;
            }
            usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
        } while((0 == busy_wait_us)||((busy_wait_us-SPI_MCU_CHECK_STATUS_DELAY_US) > 0));
        writen = (nleft > 0xFF)? 0xFF: nleft;
        put_len(s->fd, writen);
        for(i=0; i<writen; i++) {
             put_ch(s->fd, ptr[i]);
        }
        nleft -= writen;
        ptr += writen;
    }
    return (size-nleft);

SPI_MT7688_WRITE_PORT_NOT_OPEN_ERROR:
SPI_MT7688_WRITE_INPUT_ERROR:
    return ret;
}

/*
    global functions
*/
struct spi_mt7688* spi_mt7688_port_init(spi_mt7688_init_t* psp)
{
    spi_init_t* sp = NULL;
    struct spi_mt7688* ps = NULL;

    if (NULL==psp) {
        goto SPI_MT7688_PORT_INIT_INPUT_ERROR;
    }

    ps = (struct spi_mt7688*)malloc(1*sizeof(struct spi_mt7688));
    if(NULL==ps) {
        goto SPI_MT7688_PORT_INIT_MALLOC_PS_ERROR;
    }
    /* copy basic serial init values and ini basic serial */
    sp = &psp->sp;
    ps->sb = base_spi_port_init(sp);
    if(NULL==ps->sb) {
        goto SPI_MT7688_PORT_INIT_INIT_SB_ERROR;
    }
    ps->fd = 0;
    ps->open = spi_mt7688_open;
    ps->config_port = spi_mt7688_config_port;
    ps->close = spi_mt7688_close;
    ps->read = spi_mt7688_read;
    ps->write = spi_mt7688_write;
    return ps;

SPI_MT7688_PORT_INIT_INIT_SB_ERROR:
    free(ps);
SPI_MT7688_PORT_INIT_MALLOC_PS_ERROR:
SPI_MT7688_PORT_INIT_INPUT_ERROR:
    return NULL;
}
