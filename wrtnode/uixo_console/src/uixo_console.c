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

static char* uixo_console_strtok(char* src, const char* delim)
{
    static char *lasts;

    if((NULL != lasts) && (*delim == *lasts)) {
        *lasts = '\0';
        lasts++;
        return NULL;
    }
    return strtok_r(src, delim, &lasts);
}

static int _find_string_between_two_char(const char* src, const char start, const char end, char* dest)
{
    char* sptr = NULL;
    char* tmp = NULL;
    int len = 0;

    if((NULL == src) || (NULL == dest)) {
        printf("%s: input data or dest buffer is NULL\n", __func__);
        return -1;
    }
    if(strlen(src) > MAX_BUFFER_LEN) {
        printf("%s: input data is too long, MAX is %d\n", __func__, MAX_BUFFER_LEN);
        return -1;
    }

    sptr = strchr(src, start);
    if(NULL == sptr) {
        printf("%s: no %c in %s\n", __func__, start, src);
        return -1;
    }

    while((*tmp != end) && (*tmp != '\0')) {
        len++;
        tmp++;
    }
    if(*tmp == '\0') {
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

static int uixo_console_load_cmd(const char* data, const ssize_t len, uixo_message_t* msg)
{
	int ret = 0;
    char* valid_data = NULL;
    char* tmp_data = NULL;
    int i = 0;
    char* data_ptr = NULL;
    char* ptr = NULL;

    if((0 == len) || (NULL == data)) {
        return -UIXO_CONSOLE_ERR_INPUT_NULL;
    }

    valid_data = (char*)calloc(MAX_BUFFER_LEN, sizeof(*valid_data));
    tmp_data = (char*)calloc(MAX_BUFFER_LEN, sizeof(*tmp_data));
    if((NULL == valid_data) || (NULL == tmp_data)) {
        printf("%s: calloc error\n", __func__);
        return -UIXO_ERR_NULL;
    }

	/* onemsg : [time:len:cmd:data:rttimes:baudrate:/dev/device_name:fnname] */
	ret = _find_string_between_two_char(data, '[', ']', valid_data);
    if(ret < 0) {
        goto LOAD_CMD_DATA_FORMAT_ERROR;
    }
    PR_DEBUG("%s: got valid data = %s, len = %d.\n", __func__, valid_data, ret);

    memcpy(tmp_data, valid_data, ret*sizeof(*tmp_data));
    data_ptr = tmp_data;
    i = 0;
    while(i++ < 8) {
        ptr = uixo_console_strtok(data_ptr, ":");
        PR_DEBUG("%s: parse string = %s.\n", __func__, ptr);
        switch(i) {
        case 0: msg->time = (ptr != NULL)? atoi(ptr): 0; break;
        case 1: msg->len = (ptr != NULL)? atoi(ptr): 0; break;
        case 2: msg->cmd = (ptr != NULL)? *ptr: '\0'; break;
        case 3: if((msg->len != 0) && (ptr != NULL)) {
                    msg->data = (char*)calloc(msg->len, sizeof(char));
                    if(msg->data == NULL) {
                        printf("%s: message calloc error.\n", __func__);
                        return -UIXO_ERR_NULL;
                    }
                    memcpy(msg->data, ptr, msg->len);
                }
                else {
                    msg->data = NULL;
                }
                break;
        case 4: msg->rttimes = (ptr != NULL)? atoi(ptr): 0; break;
        case 5: msg->port_baudrate = (ptr != NULL)? atoi(ptr): 0; break;
        case 6: if(ptr != NULL) {
                    int port_len = strlen(ptr);
                    msg->port_name = (char*)calloc(port_len+1, sizeof(char));
                    if(msg->port_name == NULL) {
                        printf("%s: message calloc error.\n", __func__);
                        return -UIXO_ERR_NULL;
                    }
                    memcpy(msg->port_name, ptr, port_len);
                }
                else {
                    msg->port_name = NULL;
                }
                break;
        case 7: if(ptr != NULL) {
                    int port_len = strlen(ptr);
                    msg->fn_name = (char*)calloc(port_len+1, sizeof(char));
                    if(msg->fn_name == NULL) {
                        printf("%s: message calloc error.\n", __func__);
                        return -UIXO_ERR_NULL;
                    }
                    memcpy(msg->fn_name, ptr, port_len);
                }
                else {
                    printf("%s: Warning, fn name should not be empty.\n", __func__);
                    msg->fn_name = NULL;
                }
        default: printf("%s: too many parameters.\n", __func__);
                 break;
        }
        data_ptr = NULL;
    }

    free(valid_data);
    free(tmp_data);
    return 0;

LOAD_CMD_DATA_FORMAT_ERROR:
    printf("%s: message format error. message = %s.\n", __func__, data);
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
	uixo_message_t* msg = NULL;
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
int uixo_console_parse_msg(const char* data, const ssize_t len, uixo_message_t* msg)
{
	if(uixo_console_load_cmd(data, len, msg) < 0) {
		return -UIXO_CONSOLE_ERR_LOAD_CMD;
	}
	return 0;
}
