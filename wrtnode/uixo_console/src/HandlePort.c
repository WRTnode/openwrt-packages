/*
FileName    :HandlePort.c
Description :Handle serial
Author      :WRTnode machine team
Email       :summer@wrtnode.com
Version     :V01
Data        :2015.06.03
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "serial.h"
#include "serial_posix.h"
#include "spi.h"
#include "spi_mt7688.h"
#include "uixo_console.h"

/* default set serial */
static void handle_port_uixo_default_set(uixo_port_t* port, const char* port_name, const int baudrate)
{
    if(0 == strncmp(port_name, "/dev/spiS", strlen("/dev/spiS"))) {
        char* b = NULL;
        char* p = NULL;
        p = (char*)calloc(strlen(port_name)+1, sizeof(char));
        b = (char*)calloc(10, sizeof(char));
        if(NULL == b) {
            printf("%s: calloc baudrate error.\n", __func__);
            return ;
        }
        sprintf(b, "%d", baudrate);
        port->baudrate = b;
        port->bytesize = "8";
        port->timeout = "0";
        strcpy(p, port_name);
        port->name = p;
    }
    else {
        char* b = NULL;
        char* p = NULL;
        p = (char*)calloc(strlen(port_name)+1, sizeof(char));
        b = (char*)calloc(10, sizeof(char));
        if(NULL == b) {
            printf("%s: calloc baudrate error.\n", __func__);
            return ;
        }
        port->rx_head = "HY>";
        port->tx_head = "HY<";
        sprintf(b, "%d", baudrate);
        port->baudrate = b;
        port->bytesize = "8";
        port->parity = "none";
        port->stopbits = "1";
        port->timeout = "0";
        strcpy(p, port_name);
        port->name = p;
    }
}

/* open serial */
static uixo_err_t handle_port_uixo_port_open(uixo_port_t* port)
{
	uixo_err_t ret = UIXO_ERR_OK;

	if(strncmp(port->name, "/dev/tty", 8)==0) {
        posix_serial_init_t port_conf;
        struct posix_serial* ps = NULL;
		memset(&port_conf, 0, sizeof(posix_serial_init_t));
		port_conf.sp.baudrate = port->baudrate;
		port_conf.sp.bytesize = port->bytesize;
		port_conf.sp.parity   = port->parity;
		port_conf.sp.port     = port->name;
		port_conf.sp.stopbits = port->stopbits;
		port_conf.sp.timeout  = port->timeout;
		PR_DEBUG("%s,%s,%s,%s,%s,%s\n",port_conf.sp.baudrate,port_conf.sp.bytesize,port_conf.sp.parity,port_conf.sp.port,port_conf.sp.stopbits,port_conf.sp.timeout);
		ps = posix_serial_port_init(&port_conf);
		if(NULL==ps) {
			printf("Init %s port error!\n",port->name);
			return -UIXO_ERR_SERIAL_INIT_ERR;
		}
		if(ps->open(ps)<0) {
			printf("Open %s port error!\n",port->name);
			return -UIXO_ERR_SERIAL_OPEN_ERR;
		}
		port->port = ps;
	}
    else if(0 == strncmp(port->name, "/dev/spiS", strlen("/dev/spiS"))) {
        spi_mt7688_init_t port_conf;
        struct spi_mt7688* sm = NULL;
        memset(&port_conf, 0, sizeof(port_conf));
        port_conf.sp.port = port->name;
        port_conf.sp.timeout = port->timeout;
        port_conf.sp.bytesize = port->bytesize;
        port_conf.sp.baudrate = port->baudrate;
        sm = spi_mt7688_port_init(&port_conf);
        if(NULL == sm) {
             printf("%s: Init %s port error!\n", __func__, port->name);
             return -UIXO_ERR_SERIAL_OPEN_ERR;
        }
        if(sm->open(sm) < 0) {
            printf("%s: Open %s port error!\n", __func__, port->name);
            return -UIXO_ERR_SERIAL_OPEN_ERR;
        }
        port->port = sm;
        PR_DEBUG("%s: open %s OK, fd = %d\n", __func__, sm->sb->port, sm->fd);
    }
	return ret;
}

/* create a port */
uixo_port_t* handle_port_mkport(const char* port_name, const int baudrate)
{
    uixo_port_t* port = NULL;

	port = (uixo_port_t*)calloc(1, sizeof(uixo_port_t));
	if(NULL == port) {
		return NULL;
		printf("%s: Failed to calloc uixo_port_t!\n", __func__);
	}
	handle_port_uixo_default_set(port, port_name, baudrate);
	INIT_LIST_HEAD(&port->msghead);
	if(handle_port_uixo_port_open(port) < 0) {
		printf("%s: open port error\n", __func__);
		return NULL;
	}
	return port;
}

/* delete a port */
int handle_port_delport(const char* port_name, struct list_head* port_head)
{
    int has_port = 0;
	uixo_port_t* tmp_p = NULL;
	uixo_message_t* msg = NULL;

    list_for_each_entry(tmp_p, port_head, list) {
        if(strcmp(tmp_p->name, port_name) == 0) {
            has_port = 1;
            break;
        }
    }
    if(has_port) {
        PR_DEBUG("%s: start to delete port = %s.\n", __func__, tmp_p->name)
        list_del(tmp_p);
	    if(0 == strncmp(port_name, "/dev/tty", strlen("/dev/tty"))) {
            struct posix_serial* ps = tmp_p->port;

            ps->close(ps);
            free(tmp_p->baudrate);
            free(tmp_p->name);
	    }
        else if(0 == strncmp(port_name, "/dev/spiS", strlen("/dev/spiS"))) {
            struct spi_mt7688* sm = NULL;

            sm->close(sm);
            free(tmp_p->baudrate);
            free(tmp_p->name);
        }
        handle_msg_del_msglist(&tmp_p->msghead);
        free(tmp_p);
        return 0;
    }
    else {
        return -1;
    }
}
