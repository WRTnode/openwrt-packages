#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define UIXO_HEAD_LEN     (5)
#define MAX_UIXO_MSG_LEN  (4096)

#define PORT 8000
typedef struct{
	int sfd;
	int times;
}argument;

#ifdef DEBUG
#define PR_DBG   printf
#else
#define PR_DBG(...)
#endif

int client_socket_fd;

void* work(void* arg1){
	argument *arg = (argument*)arg1;
	int size;
	char buff[1024] = {0};
	int cs = arg->sfd;
	//加1，是因为发送出去一条消息会收到一条打印信息
	int rttimes = arg->times;
    fd_set sreadfd;

    PR_DBG("%s: in thread.\n", __func__);
	while(rttimes) {
        memset(buff, 0, sizeof(buff));
        FD_ZERO(&sreadfd);
        FD_SET(cs, &sreadfd);
        select(cs+1, &sreadfd, NULL, NULL, NULL);
		size = read(cs,buff,sizeof(buff));
		if(size > 0){
			rttimes--;
		}
		if(size == 0){
			break;
		}

		buff[size] = '\0';
		printf("%s",buff);
	}
    send(cs, "exit", sizeof("exit"), 0);
	close(cs);
	return 0;

}

static void quit_signal_handler(int a)
{
    send(client_socket_fd, "exit", sizeof("exit"), 0);
	close(client_socket_fd);
    exit(0);
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

    signal(SIGINT, quit_signal_handler);
    signal(SIGTSTP, quit_signal_handler);

	cs = socket(AF_INET,SOCK_STREAM,0);
	if(cs<0){
		printf("creat the socket fail!\n");
		return -1;
	}
    client_socket_fd = cs;
	memset(buff,0,sizeof(buff));
	memset(&serveraddr,0,sizeof(serveraddr));
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(PORT);
	serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
	inet_pton(AF_INET,argv[1],&serveraddr.sin_addr);
	setsockopt(cs,SOL_SOCKET,SO_REUSEADDR,&n,sizeof(int));
	ret = connect(cs,(struct sockaddr*)&serveraddr,sizeof(struct sockaddr));
	PR_DBG("ret = %d\n",ret);
	if(ret < 0){
		printf("connect error\n");
	}
#if 0
	printf("Ple input a similar [1234:2:m:22:3:115200:/dev/ttyS0:mkport],input 'quit' to exit\n");
#endif
    rttimes = atoi(argv[3]);
    if(0 != rttimes) {
	    argument* arg = NULL;
        arg = (argument*)calloc(1, sizeof(*arg));
	    arg->sfd = cs;
	    arg->times = rttimes;
	    ret = pthread_create(&tpid, NULL,work,arg) ;
	    if(ret != 0){
		    printf("pthread_create fail\n");
		    return -1;
	    }
    }
	strcpy(cstring,argv[2]);
	string_len = strlen(cstring);
	//加换行符，否则导致阻塞
	cstring[string_len] = '\n';
    cstring[string_len+1] = '\0';

#if 0
	if(strncmp(cstring,"quit",4)==0){
		printf("quit client\n");
		close(cs);
		return 0;
	}
	else if(cstring[0]!='['|| cstring[(string_len)-1]!=']')
		printf("Ple input right format as '[time:len:cmd:data:/dev/device_name]'\n");
#endif
    {
        char head[UIXO_HEAD_LEN] = {0};
        string_len = strlen(cstring);
        if(string_len > MAX_UIXO_MSG_LEN) {
            printf("%s: input string too long. len=%d.\n", __func__, string_len);
        }
        sprintf(head, "%04d", string_len);
        ret = send(cs, head, UIXO_HEAD_LEN, 0);
	    if(ret < 0){
		    printf("send error\n");
	    }
    }
	ret = send(cs,cstring,strlen(cstring),0);
	if(ret < 0){
		printf("send error\n");
	}
	if(rttimes == 0){
        send(cs, "exit", sizeof("exit"), 0);
		close(cs);
		return 0;
	}

	pthread_join(tpid, NULL);
	printf("Received all the data\n");
	close(cs);
	return 0;
}
