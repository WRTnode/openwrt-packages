/*
FileName    :HandleMsg.c
Description :Handle message
Author      :WRTnode machine team
Email       :summer@wrtnode.com
Version     :V01
Data        :2015.06.03
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/socket.h>

#include "serial.h"
#include "serial_posix.h"
#include "spi.h"
#include "spi_mt7688.h"
#include "uixo_console.h"

static int handle_msg_free_msg(uixo_message_t* msg)
{
    uixo_console_free(msg);
    return 0;
}

int handle_msg_del_msg(uixo_message_t* msg)
{
    printf("%s: not work with delete message\n", __func__);
    return 0;
}

int handle_msg_del_msglist(struct list_head* msg_head)
{
	uixo_message_t* msg_del = NULL;
    uixo_message_t* msg_del_n = NULL;
    list_for_each_entry_safe(msg_del, msg_del_n, msg_head, list) {
        list_del(&msg_del->list);
        handle_msg_free_msg(msg_del);
    }
	return 0;
}

static int handle_msg_format_data(char* dest, const char* src)
{
    int len = 0;
    if((NULL == dest) || (NULL == src)) {
        printf("%s: input NULL\n", __func__);
        return -1;
    }

    while('\0' != *src) {
        if('\\' == *src) {
            switch(*(src+1)) {
            case 'x':
            case 'X':
                {
                    int ret = 0;
                    int val = 0;
                    char tmp[3] = {0};
                    tmp[0] = *(src+2);
                    tmp[1] = *(src+3);
                    ret = sscanf(tmp, "%x", &val);
                    if(0 == ret) {
                        printf("%s: HEX format error(0x%s).\n", __func__, tmp);
                        return -1;
                    }
                    *dest++ = (char)val;
                    src += 4;
                    len++;
                }
                break;
            case '0':
                {
                    dest++;
                    len++;
                    src += 2;
                }
                break;
            default:
                {
                    *dest++ = '\\';
                    *dest++ = *(src+1);
                    len += 2;
                    src += 2;
                }
            }
        }
        else {
            *dest++ = *src++;
            len++;
        }
    }
    return len;
}

int handle_msg_transmit_data(uixo_port_t* port, uixo_message_t* msg)
{
    char* tx_data = NULL;
    int data_len = 0;

    if((NULL == port) || (NULL == port->port) || (NULL == msg)) {
        printf("%s: input data is NULL\n", __func__);
        return -1;
    }
    tx_data = (char*)uixo_console_calloc(MAX_UIXO_MSG_LEN, sizeof(char));
    if(NULL == tx_data) {
        printf("%s: calloc error\n", __func__);
        return -1;
    }
    data_len = handle_msg_format_data(tx_data, msg->data);
    if(data_len <= 0) {
        printf("%s: data len = %d\n", __func__, data_len);
        uixo_console_free(tx_data);
        return -1;
    }
    PR_DEBUG("%s: TX=%s, LEN=%d\n", __func__, tx_data, data_len);

    if(strncmp(port->name, "/dev/spiS", strlen("/dev/spiS")) == 0) {
        struct spi_mt7688* sm = (struct spi_mt7688*)(port->port);
        int writen = 0;
        PR_DEBUG("%s: send to port data = %s and len = %d\n", __func__, tx_data, data_len);
        writen = sm->write(sm, tx_data, data_len);
        if(writen < 0) {
            uixo_console_free(tx_data);
            printf("%s: send message failed\n", __func__);
            return -1;
        }
    }
    else {
        struct posix_serial* ps = (struct posix_serial*)port->port;
        int writen = 0;
        PR_DEBUG("%s: send to port data = %s and len = %d\n", __func__, tx_data, data_len);
        writen = ps->write(ps, tx_data, data_len);
        if(writen < 0) {
            uixo_console_free(tx_data);
            printf("%s: send message failed\n", __func__);
            return -1;
        }
    }
    uixo_console_free(tx_data);
    return 0;
}

static void* _handle_msg_receive_data_thread(void* arg)
{
    uixo_port_t* port = (uixo_port_t*)arg;
    uixo_message_t* msg = NULL;

    pthread_mutex_lock(&port->port_mutex);
    PR_DEBUG("%s: take port lock.\n", __func__);
    port->rx_thread_is_run = 1;
    pthread_mutex_unlock(&port->port_mutex);
    PR_DEBUG("%s: release port lock.\n", __func__);
    PR_DEBUG("%s: thread(%d) running.\n", __func__, (int)port->rx_msg_thread);
    PR_DEBUG("%s: got port(%s) in receive thread.\n", __func__, port->name);

    while(!(pthread_mutex_lock(&port->port_mutex) ||
            list_empty(&port->msghead))) {
        PR_DEBUG("%s: take port lock.\n", __func__);
        msg = list_first_entry(&port->msghead, typeof(*msg), list);
        list_del(&msg->list);
        pthread_mutex_unlock(&port->port_mutex);
        PR_DEBUG("%s: release port lock.\n", __func__);
        char* rx_data = NULL;
        rx_data = (char*)uixo_console_calloc(MAX_UIXO_MSG_LEN, sizeof(*rx_data));

        PR_DEBUG("%s: got a message from list, rttimes=%d\n", __func__, msg->rttimes);
        if(UIXO_MSG_ALWAYS_WAIT_MSG == msg->rttimes) {
            while(1) {
                int len = 0;
                len = handle_port_read_line(port, rx_data, MAX_UIXO_MSG_LEN*sizeof(*rx_data));
                if(0 != len) {
                    printf("%s", rx_data);
                    if(send(msg->socketfd, rx_data, len, 0) < 0) {
                        printf("%s: send to client error.\n", __func__);
                    }
                    PR_DEBUG("%s: send %s to client(fd=%d).\n", __func__, rx_data, msg->socketfd);
                }
                memset(rx_data, 0, MAX_UIXO_MSG_LEN*sizeof(*rx_data));
            }
        }
        else if (0 < msg->rttimes) {
            while(msg->rttimes--) {
                int len = 0;
                len = handle_port_read_line(port, rx_data, MAX_UIXO_MSG_LEN*sizeof(*rx_data));
                if(0 != len) {
                    printf("%s", rx_data);
                    if(send(msg->socketfd, rx_data, len, 0) < 0) {
                        printf("%s: send to client error.\n", __func__);
                    }
                }
                memset(rx_data, 0, MAX_UIXO_MSG_LEN*sizeof(*rx_data));
            }
            handle_msg_free_msg(msg);
        }
        else {
            printf("%s: WARNING: got rttimes=0 in rx thread. port(%s), client(%d)\n",
                   __func__, port->name, msg->socketfd);
            handle_msg_free_msg(msg);
        }
    }
    if(list_empty(&port->msghead)) {
        port->rx_thread_is_run = 0;
        pthread_mutex_unlock(&port->port_mutex);
        PR_DEBUG("%s: release port lock.\n", __func__);
        PR_DEBUG("%s: WARNING: port(%s) message list is empty.\n", __func__, port->name);
    }
    else {
        printf("%s: take port(%s) lock error.\n", __func__, port->name);
    }
    return 0;
}

int handle_msg_receive_data(uixo_port_t* port)
{
    if(pthread_create(&port->rx_msg_thread, NULL, _handle_msg_receive_data_thread, port) < 0) {
        printf("%s: create port(%s) rx message thread failed.\n", __func__, port->name);
        return -1;
    }
    return 0;
}

static int _find_string_between_two_char(const char* src, const char start, const char end, char* dest)
{
    char* sptr = NULL;
    char* eptr = NULL;
    int len = 0;

    if((NULL == src) || (NULL == dest)) {
        printf("%s: input data or dest buffer is NULL\n", __func__);
        return -1;
    }
    if(strlen(src) > MAX_UIXO_MSG_LEN) {
        printf("%s: input data is too long, MAX is %d\n", __func__, MAX_UIXO_MSG_LEN);
        return -1;
    }

    sptr = strchr(src, start);
    if(NULL == sptr) {
        printf("%s: no %c in %s\n", __func__, start, src);
        return -1;
    }
    eptr = ++sptr;

    while((*eptr != end) && (*eptr != '\0')) {
        len++;
        eptr++;
    }
    if(*eptr == '\0') {
        printf("%s: no %c in %s\n", __func__, end, src);
        return -1;
    }
    else {
        if(0 != len) {
            strncpy(dest, sptr, len);
            dest[len] = '\0';
            return len;
        }
        else {
            return -1;
        }
    }
}

static int handle_msg_parse_msg(const char* data, const ssize_t len, uixo_message_t* msg)
{
    int ret = 0;
    char* valid_data = NULL;
    char* ptr = NULL;

    if((0 == len) || (NULL == data)) {
        return -1;
    }

    valid_data = (char*)uixo_console_calloc(MAX_UIXO_MSG_LEN, sizeof(*valid_data));
    if(NULL == valid_data) {
        printf("%s: calloc error\n", __func__);
        return -UIXO_ERR_NULL;
    }

	/* onemsg : [time:len:cmd:data:rttimes:baudrate:/dev/device_name:fnname] */
    ret = _find_string_between_two_char(data, '[', ']', valid_data);
    if(ret < 0) {
        goto LOAD_CMD_DATA_FORMAT_ERROR;
    }
    PR_DEBUG("%s: got valid data = %s, len = %d.\n", __func__, valid_data, ret);

    {
        char* word = NULL;
        char* sep = ":";
        ret = 0;

        for(word = strtok(valid_data, sep);
            word;
            word = strtok(NULL, sep)) {
            PR_DEBUG("%s: strtok word = %s\n", __func__, word);
            switch(ret) {
            case 0: ret += sscanf(word, "%ld", &msg->time); break;
            case 1: ret += sscanf(word, "%d", &msg->len); break;
            case 2: ret += sscanf(word, "%c", &msg->cmd); break;
            case 3: ret += sscanf(word, "%s", msg->data); break;
            case 4: ret += sscanf(word, "%d", &msg->rttimes); break;
            case 5: ret += sscanf(word, "%d", &msg->port_baudrate); break;
            case 6: ret += sscanf(word, "%s", msg->port_name); break;
            case 7: ret += sscanf(word, "%s", msg->fn_name); break;
            default: break;
            }
        }
    }
    if(ret < 8) {
        goto LOAD_CMD_DATA_FORMAT_ERROR;
    }
    PR_DEBUG("got a valid message.\ntime=%ld,len=%d,cmd=%c,data=%s,rttimes=%d,baudrate=%d,device=%s,fn=%s\n",
             msg->time, msg->len, msg->cmd, msg->data, msg->rttimes, msg->port_baudrate,
             msg->port_name, msg->fn_name);

    uixo_console_free(valid_data);
    return 0;

LOAD_CMD_DATA_FORMAT_ERROR:
    printf("%s: message format error. message = %s.\n", __func__, valid_data);
    printf("time=%ld,len=%d,cmd=%c,data=%s,rttimes=%d,baudrate=%d,device=%s,fn=%s\n",
             msg->time, msg->len, msg->cmd, msg->data, msg->rttimes, msg->port_baudrate,
             msg->port_name, msg->fn_name);
    uixo_console_free(valid_data);
    return -UIXO_CONSOLE_ERR_INPUT_NULL;
}

int handle_msg_resolve_msg(const int fd)
{
    uixo_message_t* msg = NULL;
    char head[UIXO_HEAD_LEN] = {0};
    ssize_t readn = 0;
    int ret = 0;

    PR_DEBUG("%s: client(fd = %d) send data in.\n", __func__, fd);
    msg = (uixo_message_t*)uixo_console_calloc(1, sizeof(*msg));
    if(NULL == msg) {
        printf("%s: calloc message error.\n", __func__);
        return -1;
    }

    msg->socketfd = fd;
    readn = read(fd, head, UIXO_HEAD_LEN);
    PR_DEBUG("%s: got message head = %s, len = %ld.\n", __func__, head, readn);
    if(readn != UIXO_HEAD_LEN) {
        printf("%s: read client head error. return = %ld", __func__, readn);
        ret = -1;
        goto HANDLE_MSG_MSG_FREE_OUT;
    }

    if(0 == strcmp(head, "exit")) {
        printf("%s: read client exit message.\n", __func__);
        ret = UIXO_MSG_CLIENT_EXIT_MSG;
        goto HANDLE_MSG_MSG_FREE_OUT;
    }
    else {
        char* read_buf = NULL;
        int buf_len = atoi(head);
        read_buf = (char*)uixo_console_calloc(buf_len+1, sizeof(*read_buf));
        if(NULL == read_buf) {
            printf("%s: calloc read buffer error.\n", __func__);
            ret = -1;
            goto HANDLE_MSG_MSG_FREE_OUT;
        }
        readn = read(fd, read_buf, buf_len);
        if((readn != buf_len)||(readn == -1)) {
            printf("%s: read client fd error. return = %ld\n", __func__, readn);
            ret = -1;
            goto HANDLE_MSG_READBUF_FREE_OUT;
        }
        PR_DEBUG("%s: read data = %s, length = %ld\n", __func__, read_buf, readn);

        if(handle_msg_parse_msg(read_buf, readn, msg) != UIXO_ERR_OK) {
            printf("%s: uixo message parse err.\n", __func__);
            ret = -1;
            goto HANDLE_MSG_READBUF_FREE_OUT;
        }
        if(handle_port_fun_types(msg) < 0) {
            printf("%s: parse message error.\n", __func__);
            ret = -1;
            goto HANDLE_MSG_READBUF_FREE_OUT;
        }
HANDLE_MSG_READBUF_FREE_OUT:
        uixo_console_free(read_buf);
    }

HANDLE_MSG_MSG_FREE_OUT:
    uixo_console_free(msg);
    return ret;
}
