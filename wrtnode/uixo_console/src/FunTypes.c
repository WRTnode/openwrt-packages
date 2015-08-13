/*
FileName    :FunTypes.c
Description :Function types
Author      :WRTnode machine team
Email       :summer@wrtnode.com
Version     :V01
Data        :2015.06.03
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "uixo_console.h"

typedef struct{
	struct list_head* list;
	char* port_name;
	char Callback[20];
}pth_rx;


/* the error returned to the client */
static void error_handle(int fd,char* string)
{
	char retstr[40];
	memset(retstr,0,40);
	strcpy(retstr,string);
	write(fd,retstr,strlen(retstr));
	memset(retstr,0,40);
}

#if 0
void* ReadPort(void* arg)
{
	struct list_head* list;
	uixo_port_t* p;
	char Callback[20] = {0};
	char* port_name = NULL;
	int ret = 0;
	pth_rx *pth_rx1;
	pth_rx1 = (pth_rx*)arg;
	list = (*pth_rx1).list;
	port_name = (*pth_rx1).port_name;
	strcpy(Callback,(*pth_rx1).Callback);
	PR_DEBUG("before Callback = %s\n",Callback);
	while(1){
		list_for_each_entry(p,list,list){
			if(strcmp(p->name,port_name)==0){
		//		PR_DEBUG("p->name = %s,Callback = %s,port_name = %s \n",p->name,Callback,port_name);
				ret = uixo_rx_handler(p,Callback);
				if(ret < 0){
					printf("uixo rx handler err\n");
					return NULL;
				}
			}
		}

		if(ret == 1){
			//Receive data from the port
			PR_DEBUG("Receive data\n");
			break;
		}
		usleep(1000);
	}
	PR_DEBUG("after Callback = %s\n",Callback);
}
#endif

int FunTypes(uixo_message_t* msg)
{
	/*create a port*/
	if(strcmp(msg->fn_name, "mkport") == 0) {
		PR_DEBUG("%s: mkport, name=%s, baudrate = %d\n", __func__, msg->port_name, msg->port_baudrate);
		if(NULL == handle_port_mkport(msg->port_name, msg->port_baudrate)) {
			error_handle(msg->socketfd,"mkport error\n");
            return -1;
		}
        return 0;
    }
    /* delete a port */
    else if(strcmp(msg->fn_name, "rmport") == 0) {
        PR_DEBUG("%s: rmport %s\n", __func__, msg->port_name);
        if(handle_port_delport(msg->port_name) < 0) {
            error_handle(msg->socketfd,"the port does not exist\n");
            return -1;
        }
        return 0;
    }
    /* handle a port */
    else if(strcmp(msg->fn_name, "hlport") == 0) {
        if(handle_port_hlport(msg) < 0) {
            error_handle(msg->socketfd,"hlport error\n");
            return -1;
        }
        return 0;
    }
#if 0
    else if(strcmp(fn_name,"regwait") == 0){
        pthread_t pid;
        pth_rx pth_rx;
        char Callback[20] = {0};
		strcpy(Callback,onemsg->port_baudrate);
		list_for_each_entry(p,list,list){
			if(strcmp(p->name,onemsg->port_name)==0){
				pth_rx.list = list;
				pth_rx.port_name = onemsg->port_name;
				strcpy(pth_rx.Callback,Callback);
				ret = pthread_create(&pid,NULL,ReadPort,&pth_rx);
				usleep(1000);
				pthread_join(pid,NULL);
			}
		}
	}
#endif
    else {
        printf("%s: invalid message fn_name = %s.\n",__func__, msg->fn_name);
        return -1;
    }
}
