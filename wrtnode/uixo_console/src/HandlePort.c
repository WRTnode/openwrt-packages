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
#include <pthread.h>
#include <signal.h>

#include "serial.h"
#include "serial_posix.h"
#include "spi.h"
#include "spi_mt7688.h"
#include "uixo_console.h"

static LIST_HEAD(uixo_ports_head);

/* default set serial */
static void handle_port_uixo_default_set(uixo_port_t* port, const char* port_name, const int baudrate)
{
    if(0 == strncmp(port_name, "/dev/spiS", strlen("/dev/spiS"))) {
        char* b = NULL;
        char* p = NULL;
        p = (char*)uixo_console_calloc(strlen(port_name)+1, sizeof(char));
        b = (char*)uixo_console_calloc(10, sizeof(char));
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
        p = (char*)uixo_console_calloc(strlen(port_name)+1, sizeof(char));
        b = (char*)uixo_console_calloc(10, sizeof(char));
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
static int handle_port_uixo_port_open(uixo_port_t* port)
{
	uixo_err_t ret = UIXO_ERR_OK;

	if(strncmp(port->name, "/dev/tty", strlen("/dev/tty"))==0) {
        posix_serial_init_t port_conf;
        struct posix_serial* ps = NULL;
		memset(&port_conf, 0, sizeof(posix_serial_init_t));
		port_conf.sp.baudrate = port->baudrate;
		port_conf.sp.bytesize = port->bytesize;
		port_conf.sp.parity   = port->parity;
		port_conf.sp.port     = port->name;
		port_conf.sp.stopbits = port->stopbits;
		port_conf.sp.timeout  = port->timeout;
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
    uixo_port_t* tmp_p = NULL;

    list_for_each_entry(tmp_p, &uixo_ports_head, list) {
        PR_DEBUG("%s: find port(%s) on port list.\n", __func__, tmp_p->name);
        if(strcmp(tmp_p->name, port_name) == 0) {
            printf("%s: Port(%s) already exists\n", __func__, port_name);
            return NULL;
        }
    }

    port = (uixo_port_t*)uixo_console_calloc(1, sizeof(uixo_port_t));
    if(NULL == port) {
        return NULL;
        printf("%s: Failed to calloc uixo_port_t!\n", __func__);
    }
    handle_port_uixo_default_set(port, port_name, baudrate);
    INIT_LIST_HEAD(&port->msghead);
    pthread_mutex_init(&port->port_mutex, NULL);
    if(handle_port_uixo_port_open(port) < 0) {
        printf("%s: open port error\n", __func__);
        return NULL;
    }
    list_add_tail(&port->list, &uixo_ports_head);
    return port;
}

/* delete a port */
int handle_port_delport(const char* port_name)
{
    int has_port = 0;
	uixo_port_t* tmp_p = NULL;
	uixo_message_t* msg = NULL;

    list_for_each_entry(tmp_p, &uixo_ports_head, list) {
        if(strcmp(tmp_p->name, port_name) == 0) {
            has_port = 1;
            break;
        }
    }
    if(has_port) {
        PR_DEBUG("%s: start to delete port = %s.\n", __func__, tmp_p->name);
        list_del(&tmp_p->list);
	    if(0 == strncmp(tmp_p->name, "/dev/tty", strlen("/dev/tty"))) {
            struct posix_serial* ps = tmp_p->port;

            ps->close(ps);
            uixo_console_free(tmp_p->baudrate);
            uixo_console_free(tmp_p->name);
            PR_DEBUG("%s: finished delete port = %s.\n", __func__, port_name);
	    }
        else if(0 == strncmp(tmp_p->name, "/dev/spiS", strlen("/dev/spiS"))) {
            struct spi_mt7688* sm = NULL;

            sm->close(sm);
            uixo_console_free(tmp_p->baudrate);
            uixo_console_free(tmp_p->name);
            PR_DEBUG("%s: finished delete port = %s.\n", __func__, port_name);
        }
        PR_DEBUG("%s1.\n", __func__);
        pthread_mutex_lock(&tmp_p->port_mutex);
        if((0 != tmp_p->rx_msg_thread) || (0 == pthread_kill(tmp_p->rx_msg_thread, 0))) {
            pthread_cancel(tmp_p->rx_msg_thread);
            PR_DEBUG("%s: send cancel to rx thread(%d)\n", __func__, (int)tmp_p->rx_msg_thread);
            pthread_join(tmp_p->rx_msg_thread, NULL);
            PR_DEBUG("%s: rx thread(%d) exited.\n", __func__, (int)tmp_p->rx_msg_thread);
        }
        handle_msg_del_msglist(&tmp_p->msghead);
        PR_DEBUG("%s2.\n", __func__);
        pthread_mutex_unlock(&tmp_p->port_mutex);
        pthread_mutex_destroy(&tmp_p->port_mutex);
        uixo_console_free(tmp_p);
        return 0;
    }
    else {
        printf("%s: no port(%s) on list. delport error.\n", __func__, port_name);
        return -1;
    }
}

int handle_port_hlport(uixo_message_t* msg)
{
    uixo_port_t* port = NULL;
    list_for_each_entry(port, &uixo_ports_head, list) {
        if(strcmp(port->name, msg->port_name) == 0) {
            PR_DEBUG("%s: find port = %s.\n", __func__, msg->port_name);
            if(msg->rttimes <= UIXO_MSG_DELET_MSG) {
                PR_DEBUG("%s: del msg\n", __func__);
                if(handle_msg_del_msg(msg) < 0) {
                    printf("%s: port(%s) delet message error\n", __func__, port->name);
                    return -1;
                }
                return 0;
            }
            else {
                if(handle_msg_transmit_data(port, msg) < 0) {
                    printf("%s: port(%s) transmit data fail.\n", __func__, port->name);
                    return -1;
                }
                if(msg->rttimes != 0) {
                    uixo_message_t* msg_bak = NULL;
                    msg_bak = (uixo_message_t*)uixo_console_calloc(1, sizeof(uixo_message_t));
                    if(NULL == msg_bak) {
                        printf("%s: rttimes>0, but calloc copy message error.\n", __func__);
                        return -1;
                    }
                    memcpy(msg_bak, msg, sizeof(uixo_message_t));
                    PR_DEBUG("%s: need to receive data, add to message list\n", __func__);
                    pthread_mutex_lock(&port->port_mutex);
                    list_add_tail(&msg_bak->list, &port->msghead);
                    if(!port->rx_thread_is_run) {
                        pthread_mutex_unlock(&port->port_mutex);
                        if(0 != port->rx_msg_thread) {
                            pthread_join(port->rx_msg_thread, NULL);
                            PR_DEBUG("%s: thread(%d) exit.\n", __func__, (int)port->rx_msg_thread);
                        }
                        if(handle_msg_receive_data(port) < 0) {
                            printf("%s: port(%s) receive data fail.\n", __func__, port->name);
                            return -1;
                        }
                        PR_DEBUG("%s: client(%d) open a rx thread for port(%s).\n",
                                 __func__, msg_bak->socketfd, port->name);
                    }
                    pthread_mutex_unlock(&port->port_mutex);
                }
                return 0;
            }
        }
    }
    printf("%s: the port(%s) does not exist\n", __func__, msg->port_name);
    return -1;
}

int handle_port_read_line(uixo_port_t* port, char* rx_data, const int len)
{
    int readn = 0;
    char* ptr = rx_data;

    if((NULL == port) || (NULL == rx_data)) {
        printf("%s: input NULL.\n", __func__);
        return 0;
    }

    if(0 == strncmp(port->name, "/dev/tty", strlen("/dev/tty"))) {
        struct posix_serial* ps = port->port;
        while(0 != (len - readn)) {
            int ret = 0;
            char ch = '\0';
            ret = ps->read(ps, &ch, 1);
            if(0 < ret) {
                PR_DEBUG("%s: got ch=%c(0x%02x).\n", __func__, ch, ch);
                if('\r' == ch) {
                    continue;
                }
                *ptr = ch;
                ptr++;
                readn++;
                if('\n' == ch) {
                    return readn;
                }
            }
            else {
                usleep(10000);
                continue;
            }
        }
    }
    else if(0 == strncmp(port->name, "/dev/spiS", strlen("/dev/spiS"))) {
        struct spi_mt7688* ps = port->port;
        while(0 != (len - readn)) {
            int ret = 0;
            char ch = '\0';
            ret = ps->read(ps, &ch, 1);
            if(0 < ret) {
                PR_DEBUG("%s: got ch=%c(0x%02x).\n", __func__, ch, ch);
                if('\r' == ch) {
                    continue;
                }
                *ptr = ch;
                ptr++;
                readn++;
                if('\n' == ch) {
                    return readn;
                }
            }
            else {
                usleep(10000);
                continue;
            }
        }
    }
    else {
        printf("%s: port(%s) error.\n", __func__, port->name);
        return 0;
    }
    printf("%s: read %d data, but no Enter got.\n", __func__, readn);
    return readn;
}

void handle_port_remove_port_list(void)
{
	uixo_port_t* tmp_p = NULL;

    list_for_each_entry(tmp_p, &uixo_ports_head, list) {
        PR_DEBUG("%s: start to delete port = %s.\n", __func__, tmp_p->name);
        list_del(&tmp_p->list);
        if(0 == strncmp(tmp_p->name, "/dev/tty", strlen("/dev/tty"))) {
            struct posix_serial* ps = tmp_p->port;

            ps->close(ps);
            uixo_console_free(tmp_p->baudrate);
            uixo_console_free(tmp_p->name);
	    }
        else if(0 == strncmp(tmp_p->name, "/dev/spiS", strlen("/dev/spiS"))) {
            struct spi_mt7688* sm = NULL;

            sm->close(sm);
            uixo_console_free(tmp_p->baudrate);
            uixo_console_free(tmp_p->name);
        }
        handle_msg_del_msglist(&tmp_p->msghead);
        uixo_console_free(tmp_p);
    }
}

int handle_port_fun_types(uixo_message_t* msg)
{
	/*create a port*/
	if(strcmp(msg->fn_name, "mkport") == 0) {
		PR_DEBUG("%s: mkport, name=%s, baudrate = %d\n", __func__, msg->port_name, msg->port_baudrate);
		if(NULL == handle_port_mkport(msg->port_name, msg->port_baudrate)) {
			printf("%s: mkport error.\n", __func__);
            return -1;
		}
        return 0;
    }
    /* delete a port */
    else if(strcmp(msg->fn_name, "rmport") == 0) {
        PR_DEBUG("%s: rmport %s\n", __func__, msg->port_name);
        if(handle_port_delport(msg->port_name) < 0) {
            printf("%s: the port does not exist.\n", __func__);
            return -1;
        }
        return 0;
    }
    /* handle a port */
    else if(strcmp(msg->fn_name, "hlport") == 0) {
        if(handle_port_hlport(msg) < 0) {
            printf("%s: hlport error.\n", __func__);
            return -1;
        }
        return 0;
    }
    else {
        printf("%s: invalid message fn_name = %s.\n",__func__, msg->fn_name);
        return -1;
    }
}
