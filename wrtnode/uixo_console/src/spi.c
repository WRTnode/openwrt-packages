/*
FileName    :spi.c
Description :source code of common spi.
Author      :SchumyHao
Email       :schumy.haojl@gmail.com
Version     :V01
Date        :2015.08.01
*/

/*
    include files
*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "spi.h"

#define SPI_BAUDRATE_DEFAULT          (1000000)
#define SPI_BYTESIZE_DEFAULT          (8)
/*
   base spi member functions
*/
static int _set_is_open(struct spi_base* s)
{
    int ret = 0;
    if(NULL == s) {
        ret = -SPI_ERR_INPUT_NULL;
        goto SPI_SET_IS_OPEN_INPUT_ERROR;
    }
    s->open_flag = true;
    return 0;

SPI_SET_IS_OPEN_INPUT_ERROR:
    return ret;
}

static int _clean_is_open(struct spi_base* s)
{
    int ret = 0;
    if(NULL == s) {
        ret = -SPI_ERR_INPUT_NULL;
        goto SPI_CLEAN_IS_OPEN_INPUT_ERROR;
    }
    s->open_flag = false;
    return 0;

SPI_CLEAN_IS_OPEN_INPUT_ERROR:
    return ret;
}

static int _is_open(struct spi_base* s)
{
    int ret = 0;
    if(NULL == s) {
        ret = -SPI_ERR_INPUT_NULL;
        goto SPI_IS_OPEN_INPUT_ERROR;
    }
    return (s->open_flag)? 1: 0;

SPI_IS_OPEN_INPUT_ERROR:
    return ret;
}

static int _set_port(struct spi_base* s, const char* name)
{
    int ret = 0;
    if(NULL == s) {
        ret = -SPI_ERR_INPUT_NULL;
        goto SPI_SET_PORT_INPUT_ERROR;
    }
    //check input name
    if((NULL==name)||
            (strlen(name) <= strlen("/dev/spi"))||
            (strncmp(name, "/dev/spi", strlen("/dev/spi")))) {
        /* input name is invalid, set default name */
        name = "/dev/spiS0";
    }
    //copy name to spi_base.
    s->port = (char*)malloc((strlen(name)+1)*sizeof(char));
    if(NULL == s->port) {
        ret = -SPI_ERR_MEM;
        goto SPI_SET_PORT_MEM_ERROR;
    }
    strcpy(s->port, name);
    return 0;

SPI_SET_PORT_MEM_ERROR:
SPI_SET_PORT_INPUT_ERROR:
    return ret;
}

static const char* _get_port(struct spi_base* s)
{
    if(NULL == s) {
        goto SPI_GET_PORT_INPUT_ERROR;
    }
    return s->port;

SPI_GET_PORT_INPUT_ERROR:
    return NULL;
}

static int _clean_port(struct spi_base* s)
{
    int ret = 0;
    if(NULL == s) {
        ret = -SPI_ERR_INPUT_NULL;
        goto SPI_CLEAN_PORT_INPUT_ERROR;
    }
    if(NULL != s->port) {
        free(s->port);
        s->port = NULL;
    }
    return 0;

SPI_CLEAN_PORT_INPUT_ERROR:
    return ret;
}

static int _set_baudrate(struct spi_base* s, const char* br)
{
    int ret = 0;

    if((NULL == s) || (NULL == br)) {
        ret = -SPI_ERR_INPUT_NULL;
        goto SPI_SET_BAUDRATE_INPUT_ERROR;
    }
    //save baudrate
    s->baudrate = atoi(br);
    s->baudrate = (s->baudrate)? s->baudrate: SPI_BAUDRATE_DEFAULT;
    return 0;

SPI_SET_BAUDRATE_INPUT_ERROR:
    return ret;
}

static int _get_baudrate(struct spi_base* s)
{
    if(NULL == s) {
        goto SPI_GET_BAUDRATE_INPUT_ERROR;
    }
    return s->baudrate;

SPI_GET_BAUDRATE_INPUT_ERROR:
    return 0;
}

static int _set_bytesize(struct spi_base* s, const char* bs)
{
    int ret = 0;

    if(NULL == s) {
        ret = -SPI_ERR_INPUT_NULL;
        goto SPI_SET_BYTESIZE_INPUT_ERROR;
    }
    //save bytesize
    s->bytesize = atoi(bs);
    s->bytesize = (s->bytesize)? s->bytesize: SPI_BYTESIZE_DEFAULT;
    return 0;

SPI_SET_BYTESIZE_INPUT_ERROR:
    return ret;
}

static int _get_bytesize(struct spi_base* s)
{
    if(NULL == s) {
        goto SPI_GET_BYTESIZE_INPUT_ERROR;
    }
    return s->bytesize;

SPI_GET_BYTESIZE_INPUT_ERROR:
    return -1;
}

static int _set_timeout(struct spi_base* s, const char* t)
{
    int ret = 0;
    float time = 0;

    if(NULL == s) {
        ret = -SPI_ERR_INPUT_NULL;
        goto SPI_SET_TIMEOUT_INPUT_ERROR;
    }

    time = (NULL==t)? 0: (float)atof(t);

    s->timeout.sec = (int)time;
    s->timeout.usec = (int)(1000000*(time-(int)time));
    return 0;

SPI_SET_TIMEOUT_INPUT_ERROR:
    return ret;
}

static float _get_timeout(struct spi_base* s)
{
    float ret = 0;
    float time = 0;

    if(NULL == s) {
        ret = -SPI_ERR_INPUT_NULL;
        goto SPI_GET_TIMEOUT_INPUT_ERROR;
    }
    time = (float)s->timeout.sec + (float)s->timeout.usec/1000000.0;
    return time;

SPI_GET_TIMEOUT_INPUT_ERROR:
    return ret;
}

/*
    global functions
*/
struct spi_base* base_spi_port_init(spi_init_t* sp)
{
    struct spi_base* sb = NULL;

    if (NULL==sp) {
        goto BASE_SPI_PORT_INIT_INPUT_ERROR;
    }

    /* malloc a spi_base port */
    sb = (struct spi_base*)malloc(1*sizeof(struct spi_base));
    if(NULL==sb) {
        goto BASE_SPI_PORT_INIT_MALLOC_PS_ERROR;
    }
    /* init all spi base member functions */
    sb->set_is_open = _set_is_open;
    sb->clean_is_open = _clean_is_open;
    sb->is_open = _is_open;
    sb->set_port = _set_port;
    sb->get_port = _get_port;
    sb->clean_port = _clean_port;
    sb->set_baudrate = _set_baudrate;
    sb->get_baudrate = _get_baudrate;
    sb->set_bytesize = _set_bytesize;
    sb->get_bytesize = _get_bytesize;
    sb->set_timeout = _set_timeout;
    sb->get_timeout = _get_timeout;
    /* init all spi base member variables */
    if(sb->set_port(sb, sp->port) < 0) {
        goto BASE_SPI_PORT_INIT_MEMBER_VAL_ERROR;
    }
    if(sb->set_baudrate(sb, sp->baudrate) < 0) {
        goto BASE_SPI_PORT_INIT_MEMBER_VAL_ERROR;
    }
    if(sb->set_bytesize(sb, sp->bytesize) < 0) {
        goto BASE_SPI_PORT_INIT_MEMBER_VAL_ERROR;
    }
    if(sb->set_timeout(sb, sp->timeout) < 0) {
        goto BASE_SPI_PORT_INIT_MEMBER_VAL_ERROR;
    }
    return sb;

BASE_SPI_PORT_INIT_MEMBER_VAL_ERROR:
    free(sb);
BASE_SPI_PORT_INIT_MALLOC_PS_ERROR:
BASE_SPI_PORT_INIT_INPUT_ERROR:
    return NULL;
}
