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
//#include <list.h>
//#include <../../libubox/list.h>
//#include <libubox/list.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define PORT 8000

#include "list.h"
#include "uixo_console.h"
#include "serial.h"
#include "serial_posix.h"
#include "spi.h"
#include "spi_mt7688.h"
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(x,y)  ( x<y?x:y )
#define BACKLOG 100
#define PORT_NUM 2
#define DELCLOSEFD 0
/*
   static variables
 */
static LIST_HEAD(uixo_ports_head);
struct uixo_port_t;
struct uixo_message_list_t;

int socketfd;

/*
   static functions
 */
static int usage(const char* name)
{
    fprintf(stderr,
            "Usage: %s \n"
            "   -t string       uixo tx head\n"
            "   -r string       uixo rx head\n"
            "   -T file         uixo tx cmd file\n"
            "   -R file         uixo rx cmd file\n"
            "   -b string       port baudrate\n"
            "   -B string       port bytesize\n"
            "   -P string       port parity\n"
            "   -s string       port stopbites\n"
            "   -o string       port timeout\n"
            "   -p file         port name\n"
            "\n", name
           );
    return -1;
}

static uixo_err_t resolve_msg(int sc,struct list_head* list)
{
    uixo_err_t ret = UIXO_ERR_OK;
    if(NULL==list) {
        return -UIXO_ERR_NULL;
    }
    if((ret=uixo_resolve_msg(sc,list)) < 0) {
        printf("uixo tx handler err\n");
        return -LOOP_UIXO_TX_HANDLER_ERROR;
    }
    return ret;
}
static uixo_err_t ReadPort(struct list_head* list)
{
    uixo_err_t ret = UIXO_ERR_OK;
    uixo_port_t* port = NULL;
    if(NULL==list) {
        return -UIXO_ERR_NULL;
    }
    list_for_each_entry(port, list, list) {
        if((strncmp(port->name,"/dev/tty",8)==0) ||
           (strncmp(port->name,"/dev/spi",strlen("/dev/spi"))==0)){
            if((ret=uixo_rx_handler(port,NULL)) < 0) {
                printf("uixo rx handler err\n");
                return -LOOP_UIXO_RX_HANDLER_ERROR;
            }
        }
    }
    return ret;
}
/*
   global functions
 */
int uixo_port_close(struct list_head list){
    uixo_port_t* port;
    posix_serial_init_t port_conf;
    struct posix_serial* ps = NULL;
    list_for_each_entry(port,&list,list){
        if(ps->close(ps)<0) {
            printf("close %s port error!\n",port->name);
            return 1;
        }
        free(ps);
    }
}

static void save_leave (int signo){
        signal(SIGTERM,SIG_IGN);
        signal(SIGINT,SIG_IGN);
        close(socketfd);
}

int creat_socket(void)
{
    int ss;
    char buff[1024];
    int opt = SO_REUSEADDR;
    int ret;
    size_t size =0;
    struct sockaddr_in serveraddr;
    ss = socket(AF_INET,SOCK_STREAM,0);
    if(ss<0){
        printf("creat the socket fail!\n");
        return -1;
    }

    memset(&serveraddr,0,sizeof(serveraddr));
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_port=htons(PORT);
    serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
    /*用在多播的时候，也经常使用SO_REUSEADDR，也是为了防止机器出现意外，导致端口没有释放，而使重启后的绑定失败*/
    setsockopt(ss, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
    ret = bind(ss,(struct sockaddr*)&serveraddr,sizeof(struct sockaddr));
    if(ret == 0){
        PR_DEBUG("%d bind succes \n",PORT);
    }
    listen(ss,BACKLOG);
    return ss;
}

int main (int argc, char* argv[])
{
    int ret = 0;
    uixo_port_t* port=NULL;
    uixo_port_t* p;
    struct list_head* pos;
    struct list_head* n;
    int ch = 0;
    int sc;
    int fd_a[BACKLOG] = {0};
    int max_sockfds;
    int connct_num=0;
    int i,j;
    int r_size;
    char buff[1024];
    struct sockaddr_in clinetaddr;
    socklen_t addrlen;
    fd_set sreadfds;
    int num = 0;
    struct timeval tv = {1,0}; //1s + 0ms
    uixo_message_list_t* listmsg = NULL;
    socketfd = creat_socket();
    max_sockfds = socketfd;
    signal(SIGHUP,SIG_IGN);
    signal(SIGTERM,save_leave);

    while(1){
        addrlen = sizeof(struct sockaddr);
        FD_ZERO(&sreadfds);
        FD_SET(socketfd,&sreadfds);
        for(i=0;i<BACKLOG;i++){
            if(fd_a[i] !=0){
                FD_SET(fd_a[i],&sreadfds);
            }
        }
        ret = select(max_sockfds+1,&sreadfds,NULL,NULL,&tv);
        if(ret < 0){
            perror("select fail\n");
            break;
        }
        else if(ret == 0){
            for(i=0;i<connct_num;i++){
                ReadPort(&uixo_ports_head);
            }
            usleep(5000);
            continue;
        }
        if(FD_ISSET(socketfd,&sreadfds)){
            sc=accept(socketfd,(struct sockaddr*)&clinetaddr,&addrlen);
            PR_DEBUG("clinet accept\n");
            if(sc<0){
                continue;
            }
            /*
             * question: why BACKLOG = 5???
             */
            if(connct_num < BACKLOG){
            /*
                当有客户端断开，新客户端进来的时候connct_num不再加1，而是取代断开
                的一个，如果没有断开的就connct_num++
            */
                for(i=0;i<connct_num;i++){
                    if(fd_a[i] == 0){
                        fd_a[i] = sc;
                        break;
                    }
                }
                if(i == connct_num){
                    fd_a[connct_num++] = sc;
                }
                if(sc > max_sockfds)
                    max_sockfds = sc;
            }
            else{
                printf("max connetction arrive,exit\n");
                write(sc,"max connetction arrive,bye",27);
                close(sc);
            }
            //printf("fd_a[%d] = %d\n",connct_num-1,fd_a[connct_num-1]);
        }
        for(i=0;i<connct_num;i++){
            if(FD_ISSET(fd_a[i],&sreadfds)){
                ret = resolve_msg(fd_a[i],&uixo_ports_head);
                if(ret < 0){
                    printf("clinet[%d] close\n",i);
                    close(fd_a[i]);
                    list_for_each_entry(p,&uixo_ports_head,list){
                        del_msg(p,pos,n,NULL,fd_a[i],DELCLOSEFD);

                    }
                    FD_CLR(fd_a[i],&sreadfds);
                    fd_a[i] = 0;
                }
            }
        }
    }
    /* 7.if return, close&free work */
    uixo_port_close(uixo_ports_head);
    return ret;
}
