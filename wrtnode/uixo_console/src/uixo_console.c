/*
    FileName    :uixo_console.c
    Description :The main function of uixo_console.
    Author      :SchumyHao
    Email       :schumy.haojl@gmail.com
    Version     :V01
    Data        :2014.11.06
*/
/* Include files */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#include "serial.h"
#include "serial_posix.h"
#include "uixo_console.h"

/*
    local define
*/
enum uixo_rx_status {
    UIXO_RX_STATUS_IDLE = 0,
    UIXO_RX_STATUS_GOT_INVALID,
    UIXO_RX_STATUS_GOT_HEAD,
    UIXO_RX_STATUS_GOT_MSG
};
enum uixo_cmd_status {
    UIXO_CMD_STATUS_IDLE = 0,
    UIXO_CMD_STATUS_GOT_LEN,
    UIXO_CMD_STATUS_GOT_CMD
};

/*
    static functions
*/
static int uixo_receive_data(uixo_port_t* p, uixo_message_t** msg);
static int uixo_receive_data_process(const char ch, uixo_message_t** msg, enum uixo_rx_status* status, char* head);
static int uixo_save_cmd(const int fd, const uixo_message_t* msg);
static int uixo_invalid_receive_data_process(void* port, char* str, int size);
static int uixo_transmit_data(uixo_port_t* p, const uixo_message_t* msg);
static int uixo_transmit_data_process(char* str, const uixo_message_t* msg, char* head);
static int uixo_load_cmd(const int fd, uixo_message_t** msgp);

static int uixo_receive_data(uixo_port_t* p, uixo_message_t** msgp)
{
    int ret = 0;
    if((NULL == p)||
       (NULL == p->port)||
       (NULL == p->rx_head)) {
        ret = -UIXO_CONSOLE_ERR_INPUT_NULL;
        goto UIXO_RECEIVE_DATA_INPUT_ERROR;
    }

    /* got posix serial port */
    struct posix_serial* ps = (struct posix_serial*)p->port;

    /* read port one by one. maybe can be improve */
    static char rx_buf[MAX_UIXO_MSG_LEN];
    static int rx_pos = 0;
    char* ptr = rx_buf + rx_pos;
    int readn = ps->read(ps, ptr, 1);
    if(1!=readn) {
        ret = -UIXO_CONSOLE_ERR_READ_PORT;
        goto UIXO_RECEIVE_DATA_READ_ERROR;
    }
    rx_pos += readn;
    /* call receive data process */
    static enum uixo_rx_status status = UIXO_RX_STATUS_IDLE;
    static bool is_raw = false;
    uixo_receive_data_process(*ptr, msgp, &status, p->rx_head);
    switch(status) {
        case UIXO_RX_STATUS_IDLE: /* do not got uixo message, */
            break;
        case UIXO_RX_STATUS_GOT_HEAD: /* got uixo head, set port as raw mode */
            if(!is_raw) {
                //TODO: set raw mode
                is_raw = true;
            }
            break;
        case UIXO_RX_STATUS_GOT_MSG: /* got uixo message, restore port mode, clean rx_pos */
            if(is_raw) {
                //TODO: restore port mode
                is_raw = false;
            }
            rx_pos = 0;
            status = UIXO_RX_STATUS_IDLE;
            break;
        case UIXO_RX_STATUS_GOT_INVALID: /* send rx_buf to somewhere, clean rx_pos */
            if(is_raw) { /* got invalid data when receive cmd, not in find head */
                //TODO: restore port mode
                is_raw = false;
            }
            if(uixo_invalid_receive_data_process(ps, rx_buf, rx_pos) < 0) {
                ret = -UIXO_CONSOLE_ERR_WRITE_PORT;
                goto UIXO_RECEIVE_DATA_INVALID_DATA_PROCESS_ERROR;
            }
            rx_pos = 0;
            status = UIXO_RX_STATUS_IDLE;
            break;
        default:
            ret = -UIXO_CONSOLE_ERR_STATUS;
            goto UIXO_RECEIVE_DATA_STATUS_ERROR;
    }
    return 0;

UIXO_RECEIVE_DATA_STATUS_ERROR:
    status = UIXO_RX_STATUS_IDLE;
UIXO_RECEIVE_DATA_INVALID_DATA_PROCESS_ERROR:
UIXO_RECEIVE_DATA_READ_ERROR:
    rx_pos = 0;
UIXO_RECEIVE_DATA_INPUT_ERROR:
    return ret;
}

static int uixo_receive_data_process(const char ch, uixo_message_t** msgp, enum uixo_rx_status* status, char* head)
{
    int ret = 0;
    /* tmp message variables */
    static char          cmd = 0;
    static unsigned char len = 0;
    static char*         data = NULL;
    static int           data_pos = 0;
    static unsigned char chk = 0;
    static enum uixo_cmd_status cmd_status = UIXO_CMD_STATUS_IDLE;

    if((NULL == status)||
       (NULL == head)) {
        ret = -UIXO_CONSOLE_ERR_INPUT_NULL;
        goto UIXO_RECEIVE_DATA_PROCESS_INPUT_ERROR;
    }

    switch(*status) {
            static int head_pos = 0;/* to show the position that head now checked ok */
        case UIXO_RX_STATUS_IDLE:/* to find rx head */
            if(*(head+head_pos)==ch) { /* got valid head */
                head_pos++;
            }
            else { /* got invalid head */
                head_pos = 0;
                *status = UIXO_RX_STATUS_GOT_INVALID;
            }
            if('\0'==*(head+head_pos)) { /* got hold head */
                *status = UIXO_RX_STATUS_GOT_HEAD;
                head_pos = 0;
                /* receive cmd preper */
                cmd = 0;
                len = 0;
                data = NULL;
                data_pos = 0;
                chk = 0;
                cmd_status = UIXO_CMD_STATUS_IDLE;
            }
            break;
        case UIXO_RX_STATUS_GOT_HEAD: /* got head, to find uixo message */
            switch(cmd_status) {
                case UIXO_CMD_STATUS_IDLE:/* to find len */
                    len = (unsigned char)ch;
                    data = (char*)malloc(len*sizeof(char));
                    if(NULL==data) {
                        ret = -UIXO_CONSOLE_ERR_MEM;
                        goto UIXO_RECEIVE_DATA_PROCESS_MALLOC_MSG_DATA_ERROR;
                    }
                    chk ^= len;
                    cmd_status = UIXO_CMD_STATUS_GOT_LEN;
                    break;
                case UIXO_CMD_STATUS_GOT_LEN:/* to find cmd */
                    cmd = ch;
                    chk ^= (unsigned char)cmd;
                    cmd_status = UIXO_CMD_STATUS_GOT_CMD;
                    break;
                case UIXO_CMD_STATUS_GOT_CMD:
                    if(data_pos < len) {
                        data[data_pos++] = ch;
                        chk ^= (unsigned char)(ch);
                        break;
                    }
                    else { /* got full data, check chk byte */
                        if(chk == (unsigned char)(ch)) { /* got valid message */
                            struct timeval tv;
                            uixo_message_t* msg = NULL;
                            msg = (uixo_message_t*)malloc(1*sizeof(uixo_message_t));
                            if(NULL==msg) {
                                ret = -UIXO_CONSOLE_ERR_MEM;
                                goto UIXO_RECEIVE_DATA_PROCESS_MALLOC_MSG_ERROR;
                            }
                            msg->len = len;
                            msg->cmd = cmd;
                            msg->data = data;
                            gettimeofday(&tv, NULL);
                            msg->time = tv.tv_sec*1000 + (unsigned long)(tv.tv_usec/1000);//out
                            *msgp = msg;
                            *status = UIXO_RX_STATUS_GOT_MSG;
                        }
                        else { /* got invalid message */
                            free(data);
                            *status = UIXO_RX_STATUS_GOT_INVALID;
                        }
                    }
            }
            break;
        default:
            ret = -UIXO_CONSOLE_ERR_STATUS;
            goto UIXO_RECEIVE_DATA_PROCESS_STATUS_ERROR;
    }
    return 0;

UIXO_RECEIVE_DATA_PROCESS_STATUS_ERROR:
UIXO_RECEIVE_DATA_PROCESS_MALLOC_MSG_ERROR:
    free(data);
UIXO_RECEIVE_DATA_PROCESS_MALLOC_MSG_DATA_ERROR:
    *status = UIXO_RX_STATUS_GOT_INVALID;
UIXO_RECEIVE_DATA_PROCESS_INPUT_ERROR:
    return ret;
}

static int uixo_save_cmd(const int fd, const uixo_message_t* msg)
{
    int ret = 0;
    if((NULL == msg)||
       (NULL == msg->data)) {
        ret = -UIXO_CONSOLE_ERR_INPUT_NULL;
        goto UIXO_SAVE_CMD_INPUT_ERROR;
    }

    /* translate message to string */
    char cmd_data_buf[MAX_UIXO_MSG_LEN];
    char* ptr = cmd_data_buf;
    int pos = 0;
    pos = sprintf(ptr,"[%lu:%u:%c:",msg->time,(unsigned int)msg->len,msg->cmd);
    ptr += pos;
    memcpy(ptr, msg->data, (int)msg->len);
    pos += (int)msg->len;
    ptr += (int)msg->len;
    pos += sprintf(ptr,"]\n");
    /* write string to fd */
    write(fd, cmd_data_buf, pos);
    return 0;

UIXO_SAVE_CMD_INPUT_ERROR:
    return ret;
}

static int uixo_invalid_receive_data_process(void* port, char* str, int size)
{
    int ret = 0;
    if((NULL == port)||
       (NULL == str)) {
        ret = -UIXO_CONSOLE_ERR_INPUT_NULL;
        goto UIXO_INVALID_RECEIVE_DATA_INPUT_ERROR;
    }
    /* write string to port */
    struct posix_serial* ps = (struct posix_serial*)port;
    ps->write(ps,str,size);
    return 0;

UIXO_INVALID_RECEIVE_DATA_INPUT_ERROR:
    return ret;
}

static int uixo_transmit_data(uixo_port_t* p, const uixo_message_t* msg)
{
    int ret = 0;
    if((NULL == p)||
       (NULL == p->port)||
       (NULL == p->tx_head)||
       (NULL == msg)) {
        ret = -UIXO_CONSOLE_ERR_INPUT_NULL;
        goto UIXO_TRANSMIT_DATA_INPUT_ERROR;
    }

    /* got posix serial port */
    struct posix_serial* ps = (struct posix_serial*)p->port;
    /* call transmit data process */
    char tx_buf[MAX_UIXO_MSG_LEN];
    int writen = uixo_transmit_data_process(tx_buf, msg, p->tx_head);
    if(writen < 0) {
    	printf("make message failed\n");
        ret = -UIXO_CONSOLE_ERR_PROC_MSG;
        goto UIXO_TRANSMIT_DATA_MEG_PROCESS_ERROR;
    }
    /* set port as raw */
    //TODO: set raw mode
    /* write tx_buf to port */
    writen = ps->write(ps, tx_buf, writen);
    if(ps->drain(ps)==0)
    	printf("write %d bytes to port\n",writen);
    /* restore port mode */
    //TODO: restore port mode

    return 0;

UIXO_TRANSMIT_DATA_MEG_PROCESS_ERROR:
UIXO_TRANSMIT_DATA_INPUT_ERROR:
    return ret;
}

static int uixo_transmit_data_process(char* str, const uixo_message_t* msg, char* head)
{
    int ret = 0;
    if((NULL == str)||
       (NULL == msg)||
       (NULL == head)) {
        ret = -UIXO_CONSOLE_ERR_INPUT_NULL;
        goto UIXO_TRANSMIT_DATA_PROCESS_INPUT_ERROR;
    }

    char cmd = msg->cmd;
    char* data = msg->data;
    unsigned char len = msg->len;
    unsigned char chk = 0;
    int size = 0;
    chk ^= len;
    chk ^= (unsigned char)cmd;
    int i = len;
    while(i--) {
        chk ^= (unsigned char)data[i];
    }
    /* write string */
    size = sprintf(str,"%s%c%c",head,(char)len,cmd);
    memcpy(str+size,data,len);
    size += len;
    *(str+size) = chk;
    size ++;
    return size;

UIXO_TRANSMIT_DATA_PROCESS_INPUT_ERROR:
    return ret;
}

static int uixo_load_cmd(const int fd, uixo_message_t** msgp)
{
    int ret = 0;

    /* read cmd to cmd_data_buf */
    char cmd_data_buf[MAX_UIXO_MSG_LEN];
    int readn = 0;
    if((readn=read(fd, cmd_data_buf, MAX_UIXO_MSG_LEN)) < 0) {
        /* now, read fd until meet eof, and should not bigger than MAX_UIXO_MSG_LEN
           so, it is means fd only have one message have not read before
        */
        ret = -UIXO_CONSOLE_ERR_LOAD_CMD;
        goto UIXO_LOAD_CMD_READ_FILE_ERROR;
    }
    if(0==readn) {
        /* already read EOF */
        return 0;
    }
	#ifdef DEBUG
    printf("got %d bytes\n", readn);
	#endif
    /* malloc a message */
    uixo_message_t* msg = NULL;
    if(NULL!=msg) {
        free(msg);
        msg = NULL;
    }
    msg = (uixo_message_t*)malloc(1*sizeof(uixo_message_t));
    if(NULL==msg) {
        ret = -UIXO_CONSOLE_ERR_MEM;
        goto UIXO_LOAD_CMD_MAL_MSG_ERROR;
    }
    /* find message head '[' */
    char* ptr = NULL;
    ptr = strchr(cmd_data_buf,'[');
    if(NULL == ptr) {
    	printf("failed to find '['\n");
        ret = -UIXO_CONSOLE_ERR_LOAD_CMD;
        goto UIXO_LOAD_CMD_CMD_PROCESS_ERROR;
    }
    /* read time, len, cmd */
    int len = 0;
    if(3!=sscanf(ptr, "[%lu:%u:%c:",&msg->time,&len,&msg->cmd)) {
    	printf("failed to find time len cmd\n");
        ret = -UIXO_CONSOLE_ERR_LOAD_CMD;
        goto UIXO_LOAD_CMD_CMD_PROCESS_ERROR;
    }
    if((len >= 0) && (len <= 255)) {
        msg->len = (unsigned char)len;
    }
    /* find message end ']' */
    ptr = strchr(cmd_data_buf,']');
    if(NULL == ptr) {
    	printf("failed to find ']'\n");
        ret = -UIXO_CONSOLE_ERR_LOAD_CMD;
        goto UIXO_LOAD_CMD_CMD_PROCESS_ERROR;
    }
    /* read data */
    msg->data = (char*)malloc(msg->len*sizeof(char));
    if(NULL == msg->data) {
        ret = -UIXO_CONSOLE_ERR_MEM;
        goto UIXO_LOAD_CMD_CMD_PROCESS_ERROR;
    }
    memcpy(msg->data,ptr-msg->len,msg->len);

    *msgp = msg;
    return 0;

UIXO_LOAD_CMD_CMD_PROCESS_ERROR:
UIXO_LOAD_CMD_MAL_MSG_ERROR:
UIXO_LOAD_CMD_READ_FILE_ERROR:
    return ret;
}

/*
    global functions
*/
int uixo_rx_handler(uixo_port_t* p)
{
    int ret = 0;
    if(NULL == p) {
        ret = -UIXO_CONSOLE_ERR_INPUT_NULL;
        goto UIXO_RX_HANDLER_INPUT_ERROR;
    }

    /* receive data from port */
    static uixo_message_t* msg = NULL;
    if(uixo_receive_data(p, &msg) < 0) {
        ret = -UIXO_CONSOLE_ERR_PROC_MSG;
        goto UIXO_RX_HANDLER_DATA_PROCESS_ERROR;
    }
    if(NULL != msg) { /* got a valid message */
        if(uixo_save_cmd(p->rx_cmd_fd, msg) < 0) {
            ret = -UIXO_CONSOLE_ERR_SAVE_CMD;
            goto UIXO_RX_HANDLER_SAVE_CMD_ERROR;
        }
        /* free message */
        free(msg->data);
        free(msg);
        msg = NULL;
    }
    return 0;

UIXO_RX_HANDLER_SAVE_CMD_ERROR:
UIXO_RX_HANDLER_DATA_PROCESS_ERROR:
    if(NULL!=msg) {
        if(NULL!=msg->data) {
            free(msg->data);
        }
        free(msg);
        msg = NULL;
    }
UIXO_RX_HANDLER_INPUT_ERROR:
    return ret;
}

int uixo_tx_handler(uixo_port_t* p)
{
    int ret = 0;
    if(NULL == p) {
        ret = -UIXO_CONSOLE_ERR_INPUT_NULL;
        goto UIXO_TX_HANDLER_INPUT_ERROR;
    }

    /* read data from tx_cmd_fd */
    uixo_message_t* msg = NULL;
    if(uixo_load_cmd(p->tx_cmd_fd, &msg) < 0) {
        ret = -UIXO_CONSOLE_ERR_LOAD_CMD;
        goto UIXO_TX_HANDLER_LOAD_CMD_ERROR;
    }

    if(NULL!=msg) { /* got a valid message */
        /* translate message to port */
        if(uixo_transmit_data(p, msg) < 0) {
            ret = -UIXO_CONSOLE_ERR_PROC_MSG;
            goto UIXO_TX_HANDLER_DATA_PROCESS_ERROR;
        }
        /* free message */
        free(msg->data);
        free(msg);
    }
    return 0;

UIXO_TX_HANDLER_DATA_PROCESS_ERROR:
UIXO_TX_HANDLER_LOAD_CMD_ERROR:
    if(NULL!=msg) {
        if(NULL!=msg->data) {
            free(msg->data);
        }
        free(msg);
    }
UIXO_TX_HANDLER_INPUT_ERROR:
    return ret;
}
