/*
    FileName    :serial.c
    Description :source code of common serial.
    Author      :SchumyHao
    Email       :schumy.haojl@gmail.com
    Version     :V01
    Data        :2014.10.31
*/
/*
    include files
*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "serial.h"

/*
    serial port support values
*/
static const char* support_baudrate[] = {
//the order must be the same as serial_baud_t
    "1200",
    "2400",
    "4800",
    "9600",
    "19200",
    "38400",
    "57600",
    "115200",
    //end with "0"
    "0"
};
static const char* support_bytesize[] = {
//the order must be the same as serial_bits_t
    "7",
    "8",
    //end with "0"
    "0"
};
static const char* support_parity[] = {
//the order must be the same as serial_parity_t
    "none",
    "even",
    "odd",
    //end with "0"
    "0"
};
static const char* support_stopbits[] = {
//the order must be the same as serial_stopbits_t
    "1",
    "2",
    //end with "0"
    "0"
};

/*
    static useful functions
*/
static int find_support_pos(const char** support, const char* str)
{
    int ret = 0;
    int i = 0;

    if(NULL == support) {
        ret = -SERIAL_ERR_INPUT_NULL;
        goto SERIAL_FIND_SUPPORT_POS_INPUT_ERROR;
    }

    if(NULL != str) {
        do {
            if(!strcmp(support[i], str)) {
                break;
            }
        }
        while(strcmp(support[++i], "0"));
        if(!strcmp(support[i], "0")) {
            ret = -SERIAL_ERR_NOT_SUPPORT;
            goto SERIAL_FIND_SUPPORT_POS_SUPPORT_ERROR;
        }
    }
    else {
        ret = -SERIAL_ERR_NOT_SUPPORT;
        goto SERIAL_FIND_SUPPORT_POS_SUPPORT_ERROR;
    }


    return i;

SERIAL_FIND_SUPPORT_POS_SUPPORT_ERROR:
SERIAL_FIND_SUPPORT_POS_INPUT_ERROR:
    return ret;
}
/*
    base serial member functions
*/
static int _set_is_open(struct serial_base* s)
{
    int ret = 0;
    if(NULL == s) {
        ret = -SERIAL_ERR_INPUT_NULL;
        goto SERIAL_SET_IS_OPEN_INPUT_ERROR;
    }
    s->open_flag = true;
    return 0;

SERIAL_SET_IS_OPEN_INPUT_ERROR:
    return ret;
}

static int _clean_is_open(struct serial_base* s)
{
    int ret = 0;
    if(NULL == s) {
        ret = -SERIAL_ERR_INPUT_NULL;
        goto SERIAL_CLEAN_IS_OPEN_INPUT_ERROR;
    }
    s->open_flag = false;
    return 0;

SERIAL_CLEAN_IS_OPEN_INPUT_ERROR:
    return ret;
}

static int _is_open(struct serial_base* s)
{
    int ret = 0;
    if(NULL == s) {
        ret = -SERIAL_ERR_INPUT_NULL;
        goto SERIAL_IS_OPEN_INPUT_ERROR;
    }
    return (s->open_flag)? 1: 0;

SERIAL_IS_OPEN_INPUT_ERROR:
    return ret;
}

static int _set_port(struct serial_base* s, const char* name)
{
    int ret = 0;
    if(NULL == s) {
        ret = -SERIAL_ERR_INPUT_NULL;
        goto SERIAL_SET_PORT_INPUT_ERROR;
    }
    //check input name
    if((NULL==name)||
       (strlen(name) <= strlen("/dev/tty"))||
       (strncmp(name, "/dev/tty", strlen("/dev/tty")))) {
        /* input name is invalid, set default name */
        name = "/dev/ttyS0";
    }
    //copy name to serial_base.
    s->port = (char*)malloc((strlen(name)+1)*sizeof(char));
    if(NULL == s->port) {
        ret = -SERIAL_ERR_MEM;
        goto SERIAL_SET_PORT_MEM_ERROR;
    }
    strcpy(s->port, name);
    return 0;

SERIAL_SET_PORT_MEM_ERROR:
SERIAL_SET_PORT_INPUT_ERROR:
    return ret;
}

static const char* _get_port(struct serial_base* s)
{
    if(NULL == s) {
        goto SERIAL_GET_PORT_INPUT_ERROR;
    }
    return s->port;

SERIAL_GET_PORT_INPUT_ERROR:
    return NULL;
}

static int _clean_port(struct serial_base* s)
{
    int ret = 0;
    if(NULL == s) {
        ret = -SERIAL_ERR_INPUT_NULL;
        goto SERIAL_CLEAN_PORT_INPUT_ERROR;
    }
    if(NULL != s->port) {
        free(s->port);
        s->port = NULL;
    }
    return 0;

SERIAL_CLEAN_PORT_INPUT_ERROR:
    return ret;
}

static int _set_baudrate(struct serial_base* s, const char* br)
{
    int ret = 0;
    int pos = 0;

    if(NULL == s) {
        ret = -SERIAL_ERR_INPUT_NULL;
        goto SERIAL_SET_BAUDRATE_INPUT_ERROR;
    }
    //save baudrate if it is supported
    pos = find_support_pos(support_baudrate, br);
    if(pos < 0) {
        if(-SERIAL_ERR_NOT_SUPPORT == pos) {
            pos = (int)SERIAL_BAUD_DEFAULT;
        }
        else {
            ret = pos;
            goto SERIAL_SET_BAUDRATE_SUPPORT_ERROR;
        }
    }
    s->baudrate = (serial_baud_t)pos;
    return 0;

SERIAL_SET_BAUDRATE_SUPPORT_ERROR:
SERIAL_SET_BAUDRATE_INPUT_ERROR:
    return ret;
}

static const char* _get_baudrate(struct serial_base* s)
{
    if(NULL == s) {
        goto SERIAL_GET_BAUDRATE_INPUT_ERROR;
    }
    return support_baudrate[(int)s->baudrate];

SERIAL_GET_BAUDRATE_INPUT_ERROR:
    return NULL;
}

static const char** _get_supported_baudrate(struct serial_base* s)
{
    if(NULL == s) {
        goto SERIAL_GET_SUPPORTED_BAUDRATE_INPUT_ERROR;
    }
    return support_baudrate;

SERIAL_GET_SUPPORTED_BAUDRATE_INPUT_ERROR:
    return NULL;
}

static int _set_bytesize(struct serial_base* s, const char* bs)
{
    int ret = 0;
    int pos = 0;

    if(NULL == s) {
        ret = -SERIAL_ERR_INPUT_NULL;
        goto SERIAL_SET_BYTESIZE_INPUT_ERROR;
    }
    //save bytesize if it is supported
    pos = find_support_pos(support_bytesize, bs);
    if(pos < 0) {
        if(-SERIAL_ERR_NOT_SUPPORT == pos) {
            pos = (int)SERIAL_BITS_DEFAULT;
        }
        else {
            ret = pos;
            goto SERIAL_SET_BYTESIZE_SUPPORT_ERROR;
        }
    }
    s->bytesize = (serial_bits_t)pos;
    return 0;

SERIAL_SET_BYTESIZE_SUPPORT_ERROR:
SERIAL_SET_BYTESIZE_INPUT_ERROR:
    return ret;
}

static const char* _get_bytesize(struct serial_base* s)
{
    if(NULL == s) {
        goto SERIAL_GET_BYTESIZE_INPUT_ERROR;
    }
    return support_bytesize[(int)s->bytesize];

SERIAL_GET_BYTESIZE_INPUT_ERROR:
    return NULL;
}

static const char** _get_supported_bytesize(struct serial_base* s)
{
    if(NULL == s) {
        goto SERIAL_GET_SUPPORTED_BYTESIZE_INPUT_ERROR;
    }
    return support_bytesize;

SERIAL_GET_SUPPORTED_BYTESIZE_INPUT_ERROR:
    return NULL;
}

static int _set_parity(struct serial_base* s, const char* pa)
{
    int ret = 0;
    int pos = 0;

    if(NULL == s) {
        ret = -SERIAL_ERR_INPUT_NULL;
        goto SERIAL_SET_PARITY_INPUT_ERROR;
    }
    //save parity if it is supported
    pos = find_support_pos(support_parity, pa);
    if(pos < 0) {
        if(-SERIAL_ERR_NOT_SUPPORT == pos) {
            pos = (int)SERIAL_PARITY_DEFAULT;
        }
        else {
            ret = pos;
            goto SERIAL_SET_PARITY_SUPPORT_ERROR;
        }
    }
    s->parity = (serial_parity_t)pos;
    return 0;

SERIAL_SET_PARITY_SUPPORT_ERROR:
SERIAL_SET_PARITY_INPUT_ERROR:
    return ret;
}

static const char* _get_parity(struct serial_base* s)
{
    if(NULL == s) {
        goto SERIAL_GET_PARITY_INPUT_ERROR;
    }
    return support_parity[(int)s->parity];

SERIAL_GET_PARITY_INPUT_ERROR:
    return NULL;
}

static const char** _get_supported_parity(struct serial_base* s)
{
    if(NULL == s) {
        goto SERIAL_GET_SUPPORTED_PARITY_INPUT_ERROR;
    }
    return support_parity;

SERIAL_GET_SUPPORTED_PARITY_INPUT_ERROR:
    return NULL;
}

static int _set_stopbits(struct serial_base* s, const char* sb)
{
    int ret = 0;
    int pos = 0;

    if(NULL == s) {
        ret = -SERIAL_ERR_INPUT_NULL;
        goto SERIAL_SET_STOPBITS_INPUT_ERROR;
    }
    //save stopbits if it is supported
    pos = find_support_pos(support_stopbits, sb);
    if(pos < 0) {
        if(-SERIAL_ERR_NOT_SUPPORT == pos) {
            pos = (int)SERIAL_STOPBIT_DEFAULT;
        }
        else {
            ret = pos;
            goto SERIAL_SET_STOPBITS_SUPPORT_ERROR;
        }
    }
    s->stopbits = (serial_stopbit_t)pos;
    return 0;

SERIAL_SET_STOPBITS_SUPPORT_ERROR:
SERIAL_SET_STOPBITS_INPUT_ERROR:
    return ret;
}

static const char* _get_stopbits(struct serial_base* s)
{
    if(NULL == s) {
        goto SERIAL_GET_STOPBITS_INPUT_ERROR;
    }
    return support_stopbits[(int)s->stopbits];

SERIAL_GET_STOPBITS_INPUT_ERROR:
    return NULL;
}

static const char** _get_supported_stopbits(struct serial_base* s)
{
    if(NULL == s) {
        goto SERIAL_GET_SUPPORTED_STOPBITS_INPUT_ERROR;
    }
    return support_stopbits;

SERIAL_GET_SUPPORTED_STOPBITS_INPUT_ERROR:
    return NULL;
}

static int _set_timeout(struct serial_base* s, const char* t)
{
    int ret = 0;
    float time = 0;

    if(NULL == s) {
        ret = -SERIAL_ERR_INPUT_NULL;
        goto SERIAL_SET_TIMEOUT_INPUT_ERROR;
    }

    time = (NULL==t)? 0: (float)atof(t);

    s->timeout.sec = (int)time;
    s->timeout.usec = (int)(1000000*(time-(int)time));
    return 0;

SERIAL_SET_TIMEOUT_INPUT_ERROR:
    return ret;
}
static float _get_timeout(struct serial_base* s)
{
    float ret = 0;
    float time = 0;

    if(NULL == s) {
        ret = -SERIAL_ERR_INPUT_NULL;
        goto SERIAL_GET_TIMEOUT_INPUT_ERROR;
    }
    time = (float)s->timeout.sec + (float)s->timeout.usec/1000000.0;
    return time;

SERIAL_GET_TIMEOUT_INPUT_ERROR:
    return ret;
}

/*
    global functions
*/
struct serial_base* base_serial_port_init(serial_init_t* sp)
{
    struct serial_base* sb = NULL;

    if (NULL==sp) {
        goto BASE_SERIAL_PORT_INIT_INPUT_ERROR;
    }

    /* malloc a serial_base port */
    sb = (struct serial_base*)calloc(1, sizeof(struct serial_base));
    if(NULL==sb) {
        goto BASE_SERIAL_PORT_INIT_MALLOC_PS_ERROR;
    }
    /* init all serial base member functions */
    sb->set_is_open = _set_is_open;
    sb->clean_is_open = _clean_is_open;
    sb->is_open = _is_open;
    sb->set_port = _set_port;
    sb->get_port = _get_port;
    sb->clean_port = _clean_port;
    sb->set_baudrate = _set_baudrate;
    sb->get_baudrate = _get_baudrate;
    sb->get_supported_baudrate = _get_supported_baudrate;
    sb->set_bytesize = _set_bytesize;
    sb->get_bytesize = _get_bytesize;
    sb->get_supported_bytesize = _get_supported_bytesize;
    sb->set_parity = _set_parity;
    sb->get_parity = _get_parity;
    sb->get_supported_parity = _get_supported_parity;
    sb->set_stopbits = _set_stopbits;
    sb->get_stopbits = _get_stopbits;
    sb->get_supported_stopbits = _get_supported_stopbits;
    sb->set_timeout = _set_timeout;
    sb->get_timeout = _get_timeout;
    /* init all serial base member variables */
    if(sb->set_port(sb, sp->port) < 0) {
        goto BASE_SERIAL_PORT_INIT_MEMBER_VAL_ERROR;
    }
    if(sb->set_baudrate(sb, sp->baudrate) < 0) {
        goto BASE_SERIAL_PORT_INIT_MEMBER_VAL_ERROR;
    }
    if(sb->set_bytesize(sb, sp->bytesize) < 0) {
        goto BASE_SERIAL_PORT_INIT_MEMBER_VAL_ERROR;
    }
    if(sb->set_parity(sb, sp->parity) < 0) {
        goto BASE_SERIAL_PORT_INIT_MEMBER_VAL_ERROR;
    }
    if(sb->set_stopbits(sb, sp->stopbits) < 0) {
        goto BASE_SERIAL_PORT_INIT_MEMBER_VAL_ERROR;
    }
    if(sb->set_timeout(sb, sp->timeout) < 0) {
        goto BASE_SERIAL_PORT_INIT_MEMBER_VAL_ERROR;
    }
    /* return posix serial */
    return sb;

BASE_SERIAL_PORT_INIT_MEMBER_VAL_ERROR:
    free(sb);
BASE_SERIAL_PORT_INIT_MALLOC_PS_ERROR:
BASE_SERIAL_PORT_INIT_INPUT_ERROR:
    return NULL;
}
