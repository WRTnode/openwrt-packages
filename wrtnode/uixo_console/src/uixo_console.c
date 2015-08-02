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

static int uixo_load_cmd(const int fd,struct list_head* list);
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

/* read the client data then  assigned to the struct uixo_message_list_t */
static int uixo_load_cmd(const int fd,struct list_head* list)
{
	uixo_port_t* p;
	uixo_port_t* port;
	uixo_message_list_t* msg = NULL;
	int ret = 0;

	/* read cmd to cmd_data_buf */
	char cmd_data_buf[100]={0};
	char fn_name[20] = {0};
	memset(fn_name,0,20);
	size_t readsize = 0;
	static int cmd_len,one_cmd_len;
	/* onemsg : [time:len:cmd:data:rttimes:baudrate:/dev/device_name] */
	uixo_message_list_t* onemsg = NULL;

	char* mod_data_buf;
	mod_data_buf = (char *)malloc(4096);

	/*read stream line by socket*/
	readsize = read(fd,mod_data_buf,4096);
	if((readsize == 0)||(readsize == -1)){
		close(fd);
		return -1;
	}
	PR_DEBUG("readsize = %d ; mod_data_buf = %s\n",(int)readsize, mod_data_buf);
	if(readsize > 0) {
		cmd_len=0;
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
	/* Traversal port */
	//TraversePort(list);
	/* Traversal message */
	//TraverseMsg(list);
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
int uixo_resolve_msg(int sfd,struct list_head* list)
{
	if(uixo_load_cmd(sfd,list) < 0) {
		return -UIXO_CONSOLE_ERR_LOAD_CMD;
	}
	return 0;
}
