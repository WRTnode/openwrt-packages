#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>

#define UIXO_HEAD_LEN     (5)
#define MAX_UIXO_MSG_LEN  (4096)

#define PORT 8000

#ifdef DEBUG
#define PR_DBG   printf
#else
#define PR_DBG(...)
#endif

struct client_msg {
    char head[UIXO_HEAD_LEN];
    char data[MAX_UIXO_MSG_LEN];
};

static int socketfd;

static void quit_signal_handler(int a)
{
//    send(socketfd, "exit", sizeof("exit"), 0);
	close(socketfd);
    exit(0);
}

static int uixo_client_create_socket(const char* ip)
{
	struct sockaddr_in serveraddr;
    int cs = 0;
    int opt = SO_REUSEADDR;
	cs = socket(AF_INET,SOCK_STREAM,0);
	if(cs<0){
		printf("%s: creat the socket fail!\n", __func__);
		return -1;
	}
	//memset(&serveraddr,0,sizeof(serveraddr));
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(PORT);
	serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
	inet_pton(AF_INET, ip, &serveraddr.sin_addr);
	setsockopt(cs, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
	if(connect(cs, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr)) < 0) {
		printf("%s: connect error\n", __func__);
	}
    return cs;
}

int main(int argc,char *argv[])
{
	static int rttimes = 0;
	pthread_t tpid;

    signal(SIGINT, quit_signal_handler);
    signal(SIGTSTP, quit_signal_handler);

    socketfd = uixo_client_create_socket(argv[1]);
    rttimes = atoi(argv[3]);

    {
	    int string_len = 0;
        struct client_msg msg;

	    strcpy(msg.data, argv[2]);
	    string_len = strlen(msg.data);
        msg.data[string_len] = '\n';
        string_len++;
        msg.data[string_len] = '\0';
        if(string_len > MAX_UIXO_MSG_LEN) {
            printf("%s: input string too long. len=%d.\n", __func__, string_len);
        }
//        sprintf(msg.head, "%04d", string_len);
        if(send(socketfd, msg.data, string_len, 0) < 0) {
		    printf("%s: send %s error\n", __func__, msg.data);
	    }
    }
    {
	    if(rttimes == 0){
//            if(send(socketfd, "exit", sizeof("exit"), 0) < 0) {
//		        printf("send error\n");
//            }
		    close(socketfd);
	    }
        else {
	        int size;
	        char buff[MAX_UIXO_MSG_LEN];
            fd_set sreadfd;

	        while(rttimes) {
                FD_ZERO(&sreadfd);
                FD_SET(socketfd, &sreadfd);
                select(socketfd+1, &sreadfd, NULL, NULL, NULL);
		        size = read(socketfd, buff, sizeof(buff));
		        if(size > 0){
			        rttimes--;
                    buff[size] = '\0';
		        }
		        if(size == 0){
			        break;
		        }

		        printf("%s",buff);
	        }
//            send(socketfd, "exit", sizeof("exit"), 0);
            close(socketfd);
        }
    }

	return 0;
}
