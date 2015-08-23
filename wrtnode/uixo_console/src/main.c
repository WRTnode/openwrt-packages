/*
FileName    :main.c
Description :The main function of uixo_console.
Author      :SchumyHao
Email       :schumy.haojl@gmail.com
Version     :V03
Data        :2015.08.13
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
#include <sys/socket.h>
#include <arpa/inet.h>

#include "list.h"
#include "uixo_console.h"

#define MAX(a,b)  (((a) > (b))? (a): (b))
#define MIN(x,y)  (((x) < (y))? (x): (y))
#define PORT                         (8000)
#define BACKLOG                      (2000)

#if DugPrintg
long uixo_console_calloc_count = 0;
#endif

struct uixo_client {
    struct list_head list;
    int fd;
};

/*
   static variables
 */
static LIST_HEAD(uixo_client_head);
static int socketfd;
static int connct_num;

/*
   static functions
 */
static int usage(const char* name)
{
    return -1;
}

static void uixo_console_port_close(){
    handle_port_remove_port_list();
}

static int uixo_console_create_socket(void)
{
    int ss = 0;
    int opt = SO_REUSEADDR;
    int ret = 0;
    struct sockaddr_in serveraddr;

    ss = socket(AF_INET, SOCK_STREAM, 0);
    if(ss < 0) {
        printf("%s: creat the socket fail!\n", __func__);
        return -1;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_port=htons(PORT);
    serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
    /*用在多播的时候，也经常使用SO_REUSEADDR，也是为了防止机器出现意外，导致端口没有释放，而使重启后的绑定失败*/
    setsockopt(ss, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
    ret = bind(ss, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr));
    if(ret < 0) {
        printf("%s: bind failed.\n", __func__);
        return -1;
    }
    PR_DEBUG("%s:%d bind succes.\n",__func__, PORT);
    listen(ss, BACKLOG);
    return ss;
}

static int uixo_console_select_fds(fd_set* pfds)
{
    int max_sockfds = socketfd;
    struct uixo_client* tmp_client = NULL;

    FD_ZERO(pfds);
    FD_SET(socketfd, pfds);
    list_for_each_entry(tmp_client, &uixo_client_head, list) {
        if(0 != tmp_client->fd) {
            FD_SET(tmp_client->fd, pfds);
            max_sockfds = MAX(max_sockfds, tmp_client->fd);
            PR_DEBUG("%s: Add fd = %d to select, max_sockfds = %d.\n", __func__, tmp_client->fd, max_sockfds);
        }
    }
    return select(max_sockfds+1, pfds, NULL, NULL, NULL);
}

static int uixo_console_client_remove(const int fd)
{
    struct uixo_client* tmp_client = NULL;
    list_for_each_entry(tmp_client, &uixo_client_head, list) {
        if(fd == tmp_client->fd) {
            list_del(&tmp_client->list);
            close(tmp_client->fd);
            uixo_console_free(tmp_client);
            connct_num--;
            PR_DEBUG("%s: client removed.\n", __func__);
            return 0;
        }
    }
    printf("%s: no client(%d) in client list, removed failed.\n", __func__, fd);
    return -1;
}

static int uixo_console_handle_client(const int fd)
{
    int ret = 0;
    ret = handle_msg_resolve_msg(fd);
    if(ret < 0) {
        if(UIXO_MSG_CLIENT_EXIT_MSG == ret) {
            return uixo_console_client_remove(fd);
        }
        printf("%s: read invalid message.\n", __func__);
        return -1;
    }
    return 0;
}

static int uixo_console_handle_host(void)
{
    int sc = 0;
    struct uixo_client* client = NULL;

    if(connct_num == BACKLOG) {
        printf("%s: max connetction arrive, bye\n", __func__);
        return -1;
    }

    sc = accept(socketfd, NULL, NULL);
    if(sc < 0) {
        printf("%s: accept error.\n", __func__);
        return -1;
    }
    PR_DEBUG("%s: client accept\n", __func__);
    client = (struct uixo_client*)uixo_console_malloc(sizeof(*client));
    if(NULL == client) {
        printf("%s: client calloc error.\n", __func__);
    }
    INIT_LIST_HEAD(&client->list);
    client->fd = sc;
    list_add_tail(&client->list, &uixo_client_head);
    connct_num++;
    return 0;
}

/*
   global functions
 */
int main(int argc, char* argv[])
{
    socketfd = uixo_console_create_socket();
    if(socketfd < 0) {
        printf("%s: Bcreate socket failed.\n", __func__);
        return -1;
    }

    while(1) {
        fd_set sreadfds;
        int ret = 0;

        ret = uixo_console_select_fds(&sreadfds);
        if(ret < 0) { /* select error */
            printf("%s: select fail\n", __func__);
            break;
        }
        else if(ret == 0) { /* select timeout */
            printf("%s: Select timeout. No client send in data.\n", __func__);
            continue;
        }
        else { /* can read */
            struct uixo_client* tmp_client = NULL;
            struct uixo_client* tmp_client_next = NULL;

            list_for_each_entry_safe(tmp_client, tmp_client_next, &uixo_client_head, list) {
                PR_DEBUG("%s: Have client send data in.\n", __func__);
                if(FD_ISSET(tmp_client->fd, &sreadfds)) {
                    if(uixo_console_handle_client(tmp_client->fd) < 0) {
                        printf("%s: client handle failed.\n", __func__);
                        continue;
                    }
                }
            }
            if(FD_ISSET(socketfd, &sreadfds)) {
                PR_DEBUG("%s: host got data.\n", __func__);
                if(uixo_console_handle_host() < 0) {
                    printf("%s: hose handle failed.\n", __func__);
                    continue;
                }
            }
        }
    }
    uixo_console_port_close();
    return 0;
}
