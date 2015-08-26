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
#include <signal.h>

#include "list.h"
#include "uixo_console.h"
#include "thpool.h"

#define MAX(a,b)  (((a) > (b))? (a): (b))
#define MIN(x,y)  (((x) < (y))? (x): (y))
#define PORT                         (8000)
#define BACKLOG                      (2000)
#define THREAD_POOL_NUM              (10)

#if DugPrintg
long uixo_console_calloc_count = 0;
#endif

struct uixo_client {
    struct list_head list;
    int fd;
};

struct uixo_client_list_head {
    struct list_head head;
    pthread_rwlock_t rwlock;
    unsigned int connct_num;
};

/*
   static variables
 */
static struct uixo_client_list_head uixo_client_head;
static int socketfd;

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

static int uixo_console_host_select(void)
{
    fd_set sreadfds;
    int max_sockfds = socketfd;

    FD_ZERO(&sreadfds);
    FD_SET(socketfd, &sreadfds);
    return select(max_sockfds+1, &sreadfds, NULL, NULL, NULL);
}

static int uixo_console_client_remove(struct uixo_client* client)
{
    PR_DEBUG("%s: take client head lock.\n", __func__);
    pthread_rwlock_wrlock(&uixo_client_head.rwlock);
    list_del(&client->list);
    uixo_client_head.connct_num--;
    pthread_rwlock_unlock(&uixo_client_head.rwlock);
    PR_DEBUG("%s: release client head lock.\n", __func__);
    close(client->fd);
    uixo_console_free(client);
    PR_DEBUG("%s: client removed.\n", __func__);
    return 0;
}

static void uixo_console_handle_client(void* arg)
{
    struct uixo_client* client = (struct uixo_client*)arg;
    int ret = 0;
    printf("TIME: client(%d) start handling %d.\n", client->fd, (int)clock());
    ret = handle_msg_resolve_msg(client->fd);
    if(ret < 0) {
        printf("%s: read invalid message.\n", __func__);
    }
    PR_DEBUG("%s: removing client(%d).\n", __func__, client->fd);
    printf("TIME: client(%d) handled %d.\n", client->fd, (int)clock());
    uixo_console_client_remove(client);
}

static int uixo_console_handle_host(threadpool pool)
{
    int sc = 0;
    struct uixo_client* client = NULL;
    pid_t pid;

    PR_DEBUG("%s: take client head lock.\n", __func__);
    pthread_rwlock_rdlock(&uixo_client_head.rwlock);
    if(uixo_client_head.connct_num == BACKLOG) {
        pthread_rwlock_unlock(&uixo_client_head.rwlock);
        PR_DEBUG("%s: release client head lock.\n", __func__);
        printf("%s: max connetction arrive, bye\n", __func__);
        return -1;
    }
    pthread_rwlock_unlock(&uixo_client_head.rwlock);
    PR_DEBUG("%s: release client head lock.\n", __func__);

    sc = accept(socketfd, NULL, NULL);
    if(sc < 0) {
        printf("%s: accept error.\n", __func__);
        return -1;
    }
    PR_DEBUG("%s: client accept\n", __func__);
    printf("TIME: got client(%d) %d.\n", sc, (int)clock());
    client = (struct uixo_client*)uixo_console_malloc(sizeof(*client));
    if(NULL == client) {
        printf("%s: client calloc error.\n", __func__);
    }
    INIT_LIST_HEAD(&client->list);
    client->fd = sc;
    PR_DEBUG("%s: take client head write lock.\n", __func__);
    pthread_rwlock_wrlock(&uixo_client_head.rwlock);
    list_add_tail(&client->list, &uixo_client_head.head);
    uixo_client_head.connct_num++;
    pthread_rwlock_unlock(&uixo_client_head.rwlock);
    PR_DEBUG("%s: release client head lock.\n", __func__);

    thpool_add_work(pool, (void*)uixo_console_handle_client, client);
    PR_DEBUG("%s: add client(%d) to work queue.\n", __func__, sc);

    return 0;
}

/*
   global functions
 */
int main(int argc, char* argv[])
{
    threadpool pool;

    if(3 == argc) {
        pool = thpool_init(atoi(argv[2]));
    }
    else {
        pool = thpool_init(THREAD_POOL_NUM);
    }

    INIT_LIST_HEAD(&uixo_client_head.head);
    pthread_rwlock_init(&uixo_client_head.rwlock, NULL);
    uixo_client_head.connct_num = 0;
    handle_port_init_port_list_head();

    socketfd = uixo_console_create_socket();
    if(socketfd < 0) {
        printf("%s: Bcreate socket failed.\n", __func__);
        return -1;
    }

    while(1) {
        int ret = 0;
        ret = uixo_console_host_select();
        if(ret < 0) { /* select error */
            printf("%s: select fail\n", __func__);
            break;
        }
        else if(ret == 0) { /* select timeout */
            printf("%s: Select timeout. No client send in data.\n", __func__);
            continue;
        }
        else { /* can read */
#if 0
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
#endif
                PR_DEBUG("%s: host got data.\n", __func__);
                if(uixo_console_handle_host(pool) < 0) {
                    printf("%s: hose handle failed.\n", __func__);
                    continue;
                }
#if 0
            }
#endif
        }
    }
    uixo_console_port_close();
    thpool_wait(pool);
    thpool_destroy(pool);

    return 0;
}
