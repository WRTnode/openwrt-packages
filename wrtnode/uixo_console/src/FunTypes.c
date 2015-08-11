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

#define OTHERDEL  1

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

int FunTypes(struct list_head* port_head, uixo_message_t* msg){
	uixo_port_t* p;
	uixo_message_list_t* msg;
	int ret = 0;
	int opt1;
	int flag = 0;
	pthread_t pid;
	pth_rx pth_rx;
	char Callback[20] = {0};

	/*create a port*/
	if(strcmp(msg->fn_name, "mkport") == 0) {
        uixo_port_t* port = NULL;
        uixo_port_t* tmp_p = NULL;

		PR_DEBUG("%s: mkport, name=%s, baudrate = %d\n", __func__, msg->port_name, msg->port_baudrate);
        list_for_each_entry(tmp_p, list, list) {
            if(strcmp(tmp_p->name, msg->port_name) == 0) {
                error_handle(msg->socketfd, "Port already exists\n");
                return -1;
            }
        }
		port = handle_port_mkport(msg->port_name, msg->port_baudrate);
		if(NULL == port) {
			error_handle(msg->socketfd,"mkport error\n");
            return -1;
		}
	    list_add_tail(&port->list, port_head);
    }
	/* delete a port */
	else if(strcmp(msg->fn_name, "rmport") == 0){
		PR_DEBUG("%s: rmport %s\n", __func__, msg->port_name);
		if(handle_port_delport(msg->port_name, port_head) < 0) {
			error_handle(msg->socketfd,"the port does not exist");
            return -1;
		}
	}
	/* handle a port */
	else if(strcmp(fn_name,"hlport")==0){
		opt1 = 0;
		list_for_each_entry(p,list,list){
			if(strcmp(p->name,onemsg->port_name)==0){
				PR_DEBUG("strcmp p->msg_clinet->msg.port_name = %s\n",onemsg->port_name);
                PR_DEBUG("rttimes = %d\n",onemsg->rttimes);
				if(onemsg->rttimes == -2){
					//delete对应msg
					PR_DEBUG("del msg\n");
					opt = del_msg(p,pos,n,onemsg->port_name,onemsg->socketfd,OTHERDEL);
					if(opt == 0){
						error_handle(onemsg->socketfd,"the msg does not exist");
					}
					else{
						error_handle(onemsg->socketfd,"delect msg success");
					}
				}
				else if((onemsg->rttimes == 0) || (onemsg->rttimes < -2)){
					PR_DEBUG("not need add msglist\n");
					//不需要返回值,只执行不加入链表
					opt = uixo_transmit_data(p,onemsg);
					if(opt < 0) {
						error_handle(onemsg->socketfd,"transmit data fail");
						return -UIXO_CONSOLE_ERR_PROC_MSG;
					}
					else
						error_handle(onemsg->socketfd,"transmit data success");
				}
				else if(onemsg->rttimes == -1){
					//需要一直有返回值
					list_for_each_entry(msg,p->msghead,list){
						if(msg->cmd == onemsg->cmd){
							error_handle(onemsg->socketfd,"the cmd is working");
							flag = 1;
							break;
						}
					}
					if(flag == 0){
						//此port下没有相同的cmd在操作
						list_add_tail(&(onemsg->list),p->msghead);
						opt = uixo_transmit_data(p,onemsg);
						if(opt < 0) {
							error_handle(onemsg->socketfd,"transmit data fail");
							return -UIXO_CONSOLE_ERR_PROC_MSG;
						}
						else
							error_handle(onemsg->socketfd,"transmit data success");

					}
					flag = 0;
				}
				//加入链表并执行
				else{
					opt = uixo_transmit_data(p,onemsg);
					if(opt < 0) {
						error_handle(onemsg->socketfd,"transmit data fail");
						return -UIXO_CONSOLE_ERR_PROC_MSG;
					}
					else
						error_handle(onemsg->socketfd,"transmit data success");
					list_add_tail(&(onemsg->list),p->msghead);
				}
				opt1 = 1;
			}
		}
		if(opt1 == 0)
			error_handle(onemsg->socketfd,"the port does not exist");
	}
	else if(strcmp(fn_name,"regwait") == 0){
	//	Callback = (char*)calloc(1,20);
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
}
