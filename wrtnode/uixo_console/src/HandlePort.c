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

struct uixo_port_list_head {
    struct list_head head;
    pthread_rwlock_t rwlock;
    unsigned int port_num;
};
static struct uixo_port_list_head uixo_ports_head;

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

static uixo_port_t* find_port_on_list(const char* port_name)
{
    uixo_port_t* tmp_p = NULL;

    PR_DEBUG("%s: take port read lock.\n", __func__);
    pthread_rwlock_rdlock(&uixo_ports_head.rwlock);
    list_for_each_entry(tmp_p, &uixo_ports_head.head, list) {
        pthread_rwlock_unlock(&uixo_ports_head.rwlock);
        PR_DEBUG("%s: release port read lock.\n", __func__);
        if(strcmp(tmp_p->name, port_name) == 0) {
            return tmp_p;
        }
        PR_DEBUG("%s: take port read lock.\n", __func__);
        pthread_rwlock_rdlock(&uixo_ports_head.rwlock);
    }
    pthread_rwlock_unlock(&uixo_ports_head.rwlock);
    PR_DEBUG("%s: release port read lock.\n", __func__);

    return NULL;
}

static inline int is_port_on_list(const char* port_name)
{
    return (NULL==find_port_on_list(port_name))? 0: 1;
}

/* create a port */
uixo_port_t* handle_port_mkport(const char* port_name, const int baudrate)
{
    uixo_port_t* port = NULL;

    if(is_port_on_list(port_name)) {
        printf("%s: Port(%s) already exists\n", __func__, port_name);
    }

    port = (uixo_port_t*)uixo_console_calloc(1, sizeof(uixo_port_t));
    if(NULL == port) {
        return NULL;
        printf("%s: Failed to calloc uixo_port_t!\n", __func__);
    }
    handle_port_uixo_default_set(port, port_name, baudrate);
    pthread_mutex_init(&port->mutex, NULL);
    if(handle_port_uixo_port_open(port) < 0) {
        printf("%s: open port error\n", __func__);
        return NULL;
    }

    PR_DEBUG("%s: take port write lock.\n", __func__);
    pthread_rwlock_wrlock(&uixo_ports_head.rwlock);
    list_add_tail(&port->list, &uixo_ports_head.head);
    pthread_rwlock_unlock(&uixo_ports_head.rwlock);
    PR_DEBUG("%s: release port write lock.\n", __func__);
    return port;
}

/* delete a port */
int handle_port_delport(const char* port_name)
{
	uixo_message_t* msg = NULL;
    uixo_port_t* tmp_p = NULL;

    if((tmp_p=find_port_on_list(port_name)) == NULL) {
        printf("%s: no port(%s) on list. delport error.\n", __func__, port_name);
        return -1;
    }

    PR_DEBUG("%s: start to delete port = %s.\n", __func__, tmp_p->name);
    PR_DEBUG("%s: take port write lock.\n", __func__);
    pthread_rwlock_wrlock(&uixo_ports_head.rwlock);
    list_del(&tmp_p->list);
    pthread_rwlock_unlock(&uixo_ports_head.rwlock);
    PR_DEBUG("%s: release port write lock.\n", __func__);
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
    pthread_mutex_destroy(&tmp_p->mutex);
    uixo_console_free(tmp_p);
    return 0;
}

int handle_port_hlport(uixo_message_t* msg)
{
    uixo_port_t* port = NULL;
    char tx_data[msg->len+1];
    int data_len = 0;

    if((port=find_port_on_list(msg->port_name)) == NULL) {
        printf("%s: no port(%s) on list. hlport error.\n", __func__, port->name);
        return -1;
    }

    data_len = handle_msg_format_data(tx_data, msg->data);
    if(data_len <= 0) {
        printf("%s: data len = %d\n", __func__, data_len);
        return -1;
    }
    PR_DEBUG("%s: TX=%s, LEN=%d\n", __func__, tx_data, data_len);

    PR_DEBUG("%s: take port(%s) mutex.\n", __func__, port->name);
    pthread_mutex_lock(&port->mutex);
    if(handle_msg_transmit_data(port, tx_data, data_len) < 0) {
        printf("%s: port(%s) transmit data fail.\n", __func__, port->name);
        return -1;
    }
    if(msg->rttimes != 0) {
        if(handle_msg_receive_data(port, msg) < 0) {
            printf("%s: port(%s) receive data fail.\n", __func__, port->name);
            return -1;
        }
    }
    pthread_mutex_unlock(&port->mutex);
    PR_DEBUG("%s: release port(%s) mutex.\n", __func__, port->name);
    return 0;
}

int handle_port_read_line(uixo_port_t* port, char* rx_data, const int len)
{
    int readn = 0;
    char* ptr = rx_data;

    struct posix_serial* ps = port->port;
    readn = ps->read(ps, rx_data, 4);
    ps->flush_input(ps);
    return readn;


    if((NULL == port) || (NULL == rx_data)) {
        printf("%s: input NULL.\n", __func__);
        return 0;
    }

    if(0 == strncmp(port->name, "/dev/tty", strlen("/dev/tty"))) {
        struct posix_serial* ps = port->port;
        while(0 != (len - readn)) {
            int ret;
            char ch;
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
                    *ptr = '\0';
                    return readn;
                }
            }
            else {
                continue;
            }
        }
    }
    else if(0 == strncmp(port->name, "/dev/spiS", strlen("/dev/spiS"))) {
        struct spi_mt7688* ps = port->port;
        while(0 != (len - readn)) {
            int ret;
            char ch;
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
                    *ptr = '\0';
                    return readn;
                }
            }
            else {
                continue;
            }
        }
    }
    else {
        printf("%s: port(%s) error.\n", __func__, port->name);
        return 0;
    }
    *ptr = '\0';

    printf("%s: read %d data, but no Enter got.\n", __func__, readn);
    return readn;
}

void handle_port_remove_port_list(void)
{
	uixo_port_t* tmp_p = NULL;

    PR_DEBUG("%s: take port write lock.\n", __func__);
    pthread_rwlock_wrlock(&uixo_ports_head.rwlock);
    list_for_each_entry(tmp_p, &uixo_ports_head.head, list) {
        pthread_rwlock_unlock(&uixo_ports_head.rwlock);
        PR_DEBUG("%s: release port write lock.\n", __func__);
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
        uixo_console_free(tmp_p);
        PR_DEBUG("%s: take port write lock.\n", __func__);
        pthread_rwlock_wrlock(&uixo_ports_head.rwlock);
    }
    pthread_rwlock_unlock(&uixo_ports_head.rwlock);
    PR_DEBUG("%s: release port write lock.\n", __func__);
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

void handle_port_init_port_list_head(void)
{
    INIT_LIST_HEAD(&uixo_ports_head.head);
    pthread_rwlock_init(&uixo_ports_head.rwlock, NULL);
    uixo_ports_head.port_num = 0;
}
