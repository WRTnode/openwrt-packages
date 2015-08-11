/*
FileName    ://main.c
Description :The main function of uixo_console.
Author      :SchumyHao
Email       :schumy.haojl@gmail.com
Version     :V02
Data        :2014.12.22
 */

/* Include files */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "list.h"
#include "uixo_console.h"
#include "serial.h"
#include "serial_posix.h"
#include "spi.h"
#include "spi_mt7688.h"

#define MAX(a,b)  ((a) > (b)? (a): (b))
#define MIN(x,y)  ((x) < (y)? (x): (y))
#define BACKLOG 100
#define DELCLOSEFD 0
#define PORT 8000

/*
   static variables
 */
static LIST_HEAD(uixo_ports_head);
static LIST_HEAD(message_head);
static int socketfd;

/*
   static functions
 */
static int usage(const char* name)
{
    return -1;
}

static int uixo_console_resolve_msg(const int sc, uixo_message_t* msg)
{
    char* read_buf = NULL;
    ssize_t readn = 0;
    uixo_err_t ret = UIXO_ERR_OK;
    if(NULL == msg) {
        return -UIXO_ERR_NULL;
    }

    read_buf = (char*)calloc(MSG_BUFF_LEN, sizeof(*read_buf));
    if(NULL == read_buf) {
        printf("%s: calloc read buffer error.\n", __func__);
        return -UIXO_ERR_NULL;
    }

    msg->socketfd = sc;
    readn = read(sc, read_buf, MSG_BUFF_LEN);
    if((readn == 0)||(readn == -1)) {
        printf("%s: read client fd error. return = %ld", __func__, readn);
		return -UIXO_ERR;
	}

    PR_DEBUG("%s: read data = %s, length = %ld\n", __func__, read_buf, readn);

    if(uixo_console_parse_msg(read_buf, readn, msg) != UIXO_ERR_OK) {
        printf("%s: uixo message parse err.\n", __func__);
        return -LOOP_UIXO_TX_HANDLER_ERROR;
    }
    return UIXO_ERR_OK;
}

static int uixo_console_read_port(struct list_head* list)
{
    uixo_err_t ret = UIXO_ERR_OK;
    uixo_port_t* port = NULL;
    if(NULL==list) {
        return -UIXO_ERR_NULL;
    }
    list_for_each_entry(port, list, list) {
        if((strncmp(port->name,"/dev/tty",8)==0) ||
           (strncmp(port->name,"/dev/spi",strlen("/dev/spi"))==0)){
            ret = uixo_rx_handler(port, NULL);
            if(ret != UIXO_ERR_OK) {
                printf("uixo rx handler err\n");
                return -LOOP_UIXO_RX_HANDLER_ERROR;
            }
        }
    }
    return ret;
}

static void uixo_console_port_close(struct list_head list){
    uixo_port_t* port;
    posix_serial_init_t port_conf;
    struct posix_serial* ps = NULL;
    list_for_each_entry(port, &list, list){
        if(ps->close(ps)<0) {
            printf("%s: close %s port error!\n", __func__, port->name);
        }
        free(ps);
    }
}

static void uixo_console_save_leave (int signo){
        signal(SIGTERM,SIG_IGN);
        signal(SIGINT,SIG_IGN);
        close(socketfd);
}

static int uixo_console_creat_socket(void)
{
    int ss = 0;
    int opt = SO_REUSEADDR;
    int ret = 0;
    struct sockaddr_in serveraddr;

    ss = socket(AF_INET, SOCK_STREAM, 0);
    if(ss < 0) {
        printf("%s:creat the socket fail!\n", __func__);
        return -1;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_port=htons(PORT);
    serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
    /*用在多播的时候，也经常使用SO_REUSEADDR，也是为了防止机器出现意外，导致端口没有释放，而使重启后的绑定失败*/
    setsockopt(ss, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
    ret = bind(ss, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr));
    if(ret == 0) {
        PR_DEBUG("%s:%d bind succes.\n",__func__, PORT);
    }
    listen(ss, BACKLOG);
    return ss;
}

/*
   global functions
 */
int main(int argc, char* argv[])
{
    int ret = 0;

    socketfd = uixo_console_creat_socket();

    signal(SIGHUP, SIG_IGN);
    signal(SIGTERM, uixo_console_save_leave);

    while(1){
        int max_sockfds = socketfd;
        fd_set sreadfds;
        int fd_a[BACKLOG] = {0};
        struct timeval select_tv = {1,0}; //1s + 0ms
        int connct_num=0;
        int i = 0;

        FD_ZERO(&sreadfds);
        FD_SET(socketfd, &sreadfds);
        for(i=0; i<BACKLOG; i++) {
            if(fd_a[i] != 0) {
                FD_SET(fd_a[i],&sreadfds);
                max_sockfds = MAX(max_sockfds, fd_a[i]);
                PR_DEBUG("%s: Add fd = %d to select, max_sockfds = %d.\n", __func__, fd_a[i], max_sockfds);
            }
        }
        ret = select(max_sockfds+1, &sreadfds, NULL, NULL, &select_tv);
        if(ret < 0) { /* select error */
            printf("%s: select fail\n", __func__);
            break;
        }
        else if(ret == 0) { /* select timeout */
            PR_DEBUG("%s: Select timeout. No clinet send in data.\n", __func__);
            for(i=0; i<connct_num; i++){
                PR_DEBUG("%s: Start read port%d.\n", __func__, i);
                uixo_console_read_port(&uixo_ports_head);
                PR_DEBUG("%s: Read port%d over.\n", __func__, i);
            }
            usleep(5000);
            continue;
        }
        else { /* can read */
            PR_DEBUG("%s: Have clinet send data in.\n", __func__);
            for(i=0; i<connct_num ;i++) {
                if(FD_ISSET(fd_a[i], &sreadfds)) {
                    uixo_message_t* msg = NULL;

                    PR_DEBUG("%s: clinet(fd = %d) send data in.\n", __func__, fd_a[i]);
                    msg = (uixo_message_t*)calloc(1, sizeof(*msg));
                    if(NULL == msg) {
                        printf("%s: calloc message error.\n", __func__);
                        return -UIXO_ERR_NULL;
                    }
                    ret = uixo_console_resolve_msg(fd_a[i], msg);





                    if(ret < 0) {
                        uixo_port_t* p = NULL;
                        struct list_head* pos = 0;
                        struct list_head* n = 0;

                        close(fd_a[i]);
                        list_for_each_entry(p, &uixo_ports_head, list) {
                            del_msg(p, pos, n, NULL, fd_a[i], DELCLOSEFD);
                        }
                        FD_CLR(fd_a[i], &sreadfds);
                        fd_a[i] = 0;
                        printf("clinet[%d] close\n",i);
                    }
                }
            }
            if(FD_ISSET(socketfd, &sreadfds)) {
                int sc = 0;
                sc = accept(socketfd, NULL, NULL);
                PR_DEBUG("clinet accept\n");
                if(sc < 0) {
                    continue;
                }
                if(connct_num < BACKLOG) {
                    /*
                    当有客户端断开，新客户端进来的时候connct_num不再加1，而是取代断开
                    的一个，如果没有断开的就connct_num++
                    */
                    for(i=0; i<connct_num; i++) {
                        if(fd_a[i] == 0) {
                            fd_a[i] = sc;
                            break;
                        }
                    }
                    if(i == connct_num) {
                        fd_a[connct_num++] = sc;
                    }
                }
                else {
                    printf("max connetction arrive, bye\n");
                    write(sc, "max connetction arrive, bye\n",
                          strlen("max connetction arrive, bye\n"));
                    close(sc);
                }
            }
        }
    }
    /* 7.if return, close&free work */
    uixo_console_port_close(uixo_ports_head);
    return ret;
}
