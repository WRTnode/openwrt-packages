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
#include <regex.h>
#include <time.h>

#include "serial.h"
#include "serial_posix.h"
#include "uixo_console.h"

#define MAX_BUFFER_LEN    (4096)

static LIST_HEAD(uixo_msgs_head);

/* the error returned to the client */
void error_handle(int fd,char* string)
{
	char retstr[40];
	memset(retstr,0,40);
	strcpy(retstr,string);
	write(fd,retstr,strlen(retstr));
	memset(retstr,0,40);
}

static char* __find_string_between_two_char(const char* src, const char start, const char end)
{
}

static int _find_string_between_two_char(const char* src, const char start, const char end, char* dest)
{
    char* sptr = NULL;
    char* tmp = NULL;
    int len = 0;

    if((NULL == data) || (NULL == dest)) {
        printf("%s: input data or dest buffer is NULL\n", __func__);
        return -1;
    }
    if(strlen(data) > MAX_BUFFER_LEN) {
        printf("%s: input data is too long, MAX is %d\n", __func__, MAX_BUFFER_LEN);
        return -1;
    }

    sptr = strchr(src, start);
    if(NULL == sptr) {
        printf("%s: no %c in %s\n", __func__, start, src);
        return -1;
    }
    len++;
    tmp = sptr++;
    while((*tmp != end) && (*tmp != '\0')) {
        len++;
        tmp++;
    }
    if(*tmp == '\0') {
        printf("%s: no %c in %s\n", __func__, end, src);
        return -1;
    }
    else {
        len++;
        strncpy(dest, sptr, len);
        dest[len] = '\0';
        return len;
    }
}

static inline int _find_string_between_two_char_no_begin(const char* src, const char start, const char end, char* dest)
{
    int len = 0;
    len = _find_string_between_two_char(src, start, end, dest);
    if(len < 0) {
        return -1;
    }
    else 
}


static int uixo_console_load_cmd(const char* data, const ssize_t len, struct uixo_message_t* msg)
{
	int ret = 0;
    char* ptr = NULL;
    char* valid_data = NULL;
    char* tmp_data = NULL;

    if((0 == len) || (NULL == data)) {
        return -UIXO_CONSOLE_ERR_INPUT_NULL;
    }

    valid_data = (char*)calloc(MAX_BUFFER_LEN, sizeof(*valid_data));
    tmp_data = (char*)calloc(MAX_BUFFER_LEN, sizeof(*tmp_data));
    if((NULL == valid_data) || (NULL == tmp_data)) {
        printf("%s: calloc error\n");
        return -UIXO_ERR_NULL;
    }




typedef struct {
	struct list_head list;
	unsigned long   time;
	unsigned char   len;
	int 			timeout;
	int 			socketfd;
	int 			rttimes;
	int			    currenttime;
	char            cmd;
	char*           data;
	char* 			port_name;
	int 			port_baudrate;
} uixo_message_t;

	/* onemsg : [time:len:cmd:data:rttimes:baudrate:/dev/device_name] */
	ret = _find_string_between_two_char(data, '[', ']', valid_data);
    if(ret < 0) {
        goto LOAD_CMD_DATA_FORMAT_ERROR;
    }
    ptr = valid_data;
    PR_DEBUG("%s: got valid data = %s, len = %d.\n", __func__, valid_data, ret);

    ret = _find_string_between_two_char(ptr, '[', ':', tmp_data);
    if(ret < 0) {
        goto LOAD_CMD_DATA_FORMAT_ERROR;
    }
    msg->time = (ret <= 2)? 0: atoi(tmp_data);
    PR_DEBUG("%s: got time = %d.\n", __func__, msg->time);
    ptr += ret-1;

    ret = _find_string_between_two_char(ptr, ':', ':', tmp_data);
    if(ret < 0) {
        goto LOAD_CMD_DATA_FORMAT_ERROR;
    }
    msg->len = (ret <= 2)? 0: atoi(tmp_data);
    PR_DEBUG("%s: got len = %d.\n", __func__, msg->len);
    ptr += ret-1;

    ret = _find_string_between_two_char(ptr, ':', ':', tmp_data);
    if(ret < 0) {
        goto LOAD_CMD_DATA_FORMAT_ERROR;
    }
    msg->cmd = (ret <= 2)? '\0': tmp_data[1];
    PR_DEBUG("%s: got cmd = %c.\n", __func__, msg->cmd);
    ptr += ret-1;

    ret = _find_string_between_two_char(ptr, ':', ':', tmp_data);
    if(ret < 0) {
        goto LOAD_CMD_DATA_FORMAT_ERROR;
    }
    if(ret <= 2) {
        msg->data = NULL;
    }
    else {
        char* data_tmp = NULL;
        if(ret != msg->len) {
            printf("%s: Warning, received data length(%d) and need data length(%d) not match.\n",
                   __func__, ret, msg->len);
        }
        data_tmp = (char*)malloc((ret))
        memcpy()
    }
    PR_DEBUG("%s: got cmd = %c.\n", __func__, msg->cmd);
    ptr += ret-1;


    /* \n for the end of a command to split the received data */
		while(cmd_len<readsize){
			one_cmd_len=cmd_len;
			for(;cmd_len<4096;cmd_len++){
				if(*(mod_data_buf+cmd_len)=='\n'){
					memcpy(cmd_data_buf,mod_data_buf+one_cmd_len,cmd_len-one_cmd_len);
					break;
				}
			}
			cmd_data_buf[cmd_len-one_cmd_len]='\0';
			cmd_len++;
			if(cmd_len>4096){
				break;
			}
			/* 1.malloc a message */
			onemsg = (uixo_message_list_t*)calloc(1, sizeof(uixo_message_list_t));
			/* translate cmd to msg
			   cmd is like [time:len:cmd:data:rttimes:baudrate:/dev/device_name:fnname]
			 */
			uixo_parse_string(cmd_data_buf,onemsg,readsize,fn_name);
			onemsg->socketfd = fd;
			/* Function types ,contain of mkport,rmport,hlport*/
			FunTypes(list,onemsg,fn_name);
		}
		free(mod_data_buf);
	}




    free(valid_data);
    free(tmp_data);
    return 0;

LOAD_CMD_DATA_FORMAT_ERROR:
    printf("%s: message format error. message = %s.\n"__func__, data);
    return -UIXO_CONSOLE_ERR_INPUT_NULL;
}

/*
	handle the data from port
*/
int uixo_rx_handler(uixo_port_t* p,char* Callback)
{
	int ret = 0;
	if(NULL == p) {
		return -UIXO_CONSOLE_ERR_INPUT_NULL;
	}
	/* receive data from port */
	static uixo_message_list_t* msg = NULL;
	if(uixo_receive_data(p, &msg) < 0) {

		ret = UIXO_CONSOLE_ERR_PROC_MSG;
		goto UIXO_RX_HANDLER_DATA_PROCESS_ERROR;
	}
	if(NULL != msg) { /* got a valid message */
		ret = uixo_save_cmd(p,msg,Callback);
		if(ret < 0){
			ret = UIXO_CONSOLE_ERR_SAVE_CMD;
			goto UIXO_RX_HANDLER_SAVE_CMD_ERROR;
		}
		/* free message */
		free(msg->data);
		free(msg);
		msg = NULL;
	}
	return ret;

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

int uixo_shell(char * data){
	FILE *fstream=NULL;
	char buff[4096];
	memset(buff,0,sizeof(buff));
	if(NULL==(fstream=popen(data,"r"))) {
		printf("execute command failed");
		return -1;
	}
	if(NULL!=fgets(buff, sizeof(buff), fstream)){
		printf("%s",buff);
	} else {
		pclose(fstream);
		return -1;
	}
	pclose(fstream);
	return 0;
}

/* handle the data from client */
int uixo_console_parse_msg(const char* data, const ssize_t len, struct uixo_message_t* msg)
{
	if(uixo_console_load_cmd(data, len, msg) < 0) {
		return -UIXO_CONSOLE_ERR_LOAD_CMD;
	}
	return 0;
}
