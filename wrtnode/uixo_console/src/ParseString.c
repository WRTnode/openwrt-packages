/*
FileName    :ParseString.c
Description :Parsing the string
Author      :WRTnode machine team
Email       :summer@wrtnode.com
Version     :V01
Data        :2015.06.03
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "uixo_console.h"

int uixo_parse_string(char* cmd_data_buf,uixo_message_list_t* onemsg,int readn,char* fn_name)
{
	uixo_message_list_t* msg = NULL;

	int ret = 0;
	char dev_name[20]={0};
	/* 1.malloc a message */
	memset(onemsg->port_name,0,20);
	memset(onemsg->port_baudrate,0,20);

	const static char* delim = ":";
	char* Nptr = cmd_data_buf;
	char* ptr = cmd_data_buf;
	char* token = NULL;
	int i;
	for(i=8 ; i>0; i--,ptr=NULL) {
		token = strtok(ptr, delim);
		if(NULL == token) {
			break;
		}
		switch(i) {
			case 1: //fn_name
				memcpy(fn_name, token,((strlen(token)<20?strlen(token):20)-1));
				//PR_DEBUG("%d ,fn = %s\n",i,fn_name);
				break;

			case 2: //port_name
				memcpy(onemsg->port_name, token, 20);
				break;
			case 3: //port_baudrate
				memcpy(onemsg->port_baudrate, token, 20);
				break;
			case 4: //rttimes [0,1]  需要返回次数
				onemsg->rttimes = atoi(token);
				break;
			case 5: //data
				onemsg->data = (char*)malloc(onemsg->len+1);
				memset(onemsg->data,0,onemsg->len+1);
				if(NULL == onemsg->data) {
					return -UIXO_CONSOLE_ERR_MEM;
				}
				memcpy(onemsg->data, token, onemsg->len);

				//PR_DEBUG("Parsing a string data =%s\n",onemsg->data);
				break;
			case 6: //cmd
				onemsg->cmd = *token;
				break;
			case 7: //len
				onemsg->len = atoi(token);
				break;
			case 8: //time
				onemsg->time = strtoul(token+1, NULL, 10);
				break;
		}

	}
	PR_DEBUG("one message of rttimes = %d,data = %s,device_name = %s,fnname = %s\n",onemsg->rttimes,onemsg->data,onemsg->port_name,fn_name);
	return 0;
}

