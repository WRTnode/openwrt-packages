#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include<sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8000
typedef struct{
	int sfd;
	int times;
}argument;

void* work(void* arg1){
	argument *arg;
	arg = (argument*)arg1;
	int size;
	char buff[1024];
	int cs = (*arg).sfd;
	//加1，是因为发送出去一条消息会收到一条打印信息
	int rttimes = (*arg).times + 1;

	for(;;){
		size = read(cs,buff,sizeof(buff));
		if(size > 0){
			rttimes--;	
		}
		if(size == 0){
			break;
		}
		if(buff[0]== '\0')
			continue;

		buff[size] = '\0';
		printf("%s\n",buff);
		if(rttimes == 0){
			close(cs);
			return 0;
		}
	}
	close(cs);
	return 0;

}
int main(int argc,char *argv[])
{
	int cs,cc;
	int n;
	int ret;
	int rttimes = 0;
	char cstring[1024];
	int string_len = 0;
	char buff[1024];
	size_t size;
	pid_t pid;
	pthread_t tpid;
	struct sockaddr_in serveraddr;
	cs = socket(AF_INET,SOCK_STREAM,0);
	if(cs<0){
		printf("creat the socket fail!\n");
		return -1;
	}
	memset(buff,0,sizeof(buff));
	memset(&serveraddr,0,sizeof(serveraddr));
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(PORT);
	serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
	inet_pton(AF_INET,argv[1],&serveraddr.sin_addr);
	setsockopt(cs,SOL_SOCKET,SO_REUSEADDR,&n,sizeof(int));
	ret = connect(cs,(struct sockaddr*)&serveraddr,sizeof(struct sockaddr));
	printf("ret = %d\n",ret);
	if(ret < 0){
		printf("connect error\n");	
	}
	printf("Ple input a similar [1234:2:m:22:3:115200:/dev/ttyS0:mkport],input 'quit' to exit\n");
	rttimes = atoi(argv[3]);
	argument arg;
	arg.sfd = cs;
	arg.times = rttimes;
	ret = pthread_create(&tpid, NULL,work,&arg) ;
	if(ret != 0){
		printf("pthread_create fail\n");	
		return -1;
	}
	strcpy(cstring,argv[2]);
	string_len = strlen(cstring);
	//加换行符，否则导致阻塞
	cstring[string_len] = '\n'; 

	if(strncmp(cstring,"quit",4)==0){
		printf("quit client\n");
		close(cs);
		return 0;
	}
	else if(cstring[0]!='['|| cstring[(string_len)-1]!=']')
		printf("Ple input right format as '[time:len:cmd:data:/dev/device_name]'\n");

	ret = send(cs,cstring,strlen(cstring),0);
	if(ret < 0){
		printf("send error\n");	
	}
	if(rttimes == 0){
		close(cs);
		return 0;
	}

	pthread_join(tpid, NULL); 
	printf("Received all the data\n");
	close(cs);
	return 0;
}			
