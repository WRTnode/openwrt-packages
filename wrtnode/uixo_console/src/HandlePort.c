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
void uixo_default_set(uixo_port_t* port,char* port_name,char* baudrate)
{
    if(0 == strncmp(port_name, "/dev/spiS", strlen("/dev/spiS"))) {
        if(NULL == port->baudrate) {
            port->baudrate = baudrate;
        }
        if(NULL == port->bytesize) {
            port->bytesize = "8";
        }
        if(NULL == port->timeout) {
            port->timeout = "0";
        }
        if(NULL == port->name) {
            port->name = port_name;
        }
    }
    else {
    	if(NULL == port->rx_head) {
    		port->rx_head = "HY>";
    	}
    	if(NULL == port->tx_head) {
    		port->tx_head = "HY<";
    	}
    	if(NULL == port->baudrate) {
    		port->baudrate = baudrate;
    	}
    	if(NULL == port->bytesize) {
    		port->bytesize = "8";
    	}
    	if(NULL == port->parity) {
    		port->parity = "none";
    	}
    	if(NULL == port->stopbits) {
    		port->stopbits = "1";
    	}
    	if(NULL == port->timeout) {
    		port->timeout = "0.01";
    	}
    	if(NULL == port->name) {
    		port->name = port_name;
    	}
    }
}

/* open serial */
uixo_err_t uixo_port_open(uixo_port_t* port)
{
	uixo_err_t ret = UIXO_ERR_OK;

	if(strncmp(port->name,"/dev/tty",8)==0){
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
             printf("Init %s port error!\n", port->name);
             return -UIXO_ERR_SERIAL_OPEN_ERR;
        }
        if(sm->open(sm) < 0) {
            printf("Open %s port error!\n", port->name);
            return -UIXO_ERR_SERIAL_OPEN_ERR;
        }
        port->port = sm;
        printf("open %s OK, fd = %d\n", sm->sb->port, sm->fd);
    }
	return ret;
}
/* create a port */
int mkport(uixo_port_t* port,char* port_name,char* baudrate,struct list_head* list)
{
	uixo_port_t* p;
	int ret;
	list_for_each_entry(p,list,list){
		if(strcmp(p->name,port_name) == 0){
			printf("Serial already exists\n");
			return -2;
		}
		else{
			continue;
		}
	}
	port = (uixo_port_t*)calloc(1, sizeof(uixo_port_t));
	if(NULL==port) {
		return -1;
		perror("Failed to calloc uixo_port_t!\n");
	}
	uixo_default_set(port,port_name,baudrate);
	port->msghead = (struct list_head*)calloc(1, sizeof(*(port->msghead)));
	INIT_LIST_HEAD(port->msghead);
	list_add_tail(&port->list, list);
	ret = uixo_port_open(port);
	if(ret < 0){
		printf("open port error\n");
		return -1;
	}
	return ret;
}

/* delete a port */
int del_port(struct list_head* pos,struct list_head* n,char* port_name,struct list_head* list)
{
	int opt = 0;
	uixo_port_t* p;
	uixo_message_list_t* msg_del;
	list_for_each_safe(pos,n,list)
	{
		p = list_entry(pos,uixo_port_t,list);
		if(strcmp(p->name,port_name)==0){
			del_msglist(p,pos,n);
			list_del_init(pos);
			free(p);
			opt = 1;
		}
	}
	return opt;
}
/* delete a message list */
int del_msglist(uixo_port_t* p,struct list_head* pos,struct list_head* n)
{
	PR_DEBUG("del msg list\n");
	uixo_message_list_t* msg_del;
		list_for_each_safe(pos,n,p->msghead)
		{
			msg_del = list_entry(pos,uixo_message_list_t,list);
			list_del_init(pos);
			free(msg_del);
		}
	return 0;
}
/* delete a message */
int del_msg(uixo_port_t* p,struct list_head* pos,struct list_head* n,char* msg_name,int socketfd,int flag)
{
	int opt = 0;
	uixo_message_list_t* msg_del;
	list_for_each_safe(pos,n,p->msghead)
	{
		msg_del = list_entry(pos,uixo_message_list_t,list);
		if(flag == 0){
			if(msg_del->socketfd == socketfd){
				list_del_init(pos);
				free(msg_del);
				opt = 1;
			}
		}
		else{
			if(strcmp(msg_del->port_name,msg_name) == 0 &&\
					(msg_del->socketfd == socketfd )){
				list_del_init(pos);
				free(msg_del);
				opt = 1;
				break;
			}
			//只删第一个满足条件的msg
		}
	}
	return opt;
}

