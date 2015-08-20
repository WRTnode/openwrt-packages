/*
    FileName    :uixo_console.h
    Description :head file of uixo console.
    Author      :SchumyHao
    Email       :schumy.haojl@gmail.com
    Version     :V01
    Data        :2014.11.06
*/
#ifndef __UIXO_CONSOLE_H__
#define __UIXO_CONSOLE_H__
/*
    include files
*/
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include "list.h"

/*
    const define
*/
#define MAX_UIXO_MSG_LEN       (4096)
#define UIXO_HEAD_LEN          (5)
/*

*/
#define DugPrintg  1
#if DugPrintg
#define PR_DEBUG(fmt,args...) printf(fmt,##args)
extern long calloc_count;
#else
#define PR_DEBUG(fmt,args...) /*do nothing */
#endif

#define UIXO_MSG_ALWAYS_WAIT_MSG   (-1)
#define UIXO_MSG_DELET_MSG         (-2)
#define UIXO_MSG_CLIENT_EXIT_MSG   (-3)

typedef enum {
	UIXO_ERR_OK = 0,
    UIXO_CONSOLE_ERR_INPUT_NULL = 1,
    UIXO_CONSOLE_ERR_READ_PORT,
    UIXO_CONSOLE_ERR_WRITE_PORT,
    UIXO_CONSOLE_ERR_SAVE_CMD,
    UIXO_CONSOLE_ERR_LOAD_CMD,
    UIXO_CONSOLE_ERR_PROC_MSG,
    UIXO_CONSOLE_ERR_STATUS,
    UIXO_CONSOLE_ERR_MEM,
	UIXO_ERR_NULL,
	LOOP_UIXO_RX_HANDLER_ERROR,
	LOOP_UIXO_TX_HANDLER_ERROR,
	UIXO_ERR_FIFO,
	UIXO_ERR_SERIAL_INIT_ERR,
	UIXO_ERR_SERIAL_OPEN_ERR,
	UIXO_ERR_SELECT,
    UIXO_ERR,
} uixo_err_t;

typedef struct {
    struct list_head list;
    char            cmd;
    int             len;
    int             timeout;
    int             socketfd;
    int             rttimes;
    int             port_baudrate;
    unsigned long   time;
    unsigned long   currenttime;
    char            data[MAX_UIXO_MSG_LEN];
    char            port_name[MAX_UIXO_MSG_LEN];
    char            fn_name[MAX_UIXO_MSG_LEN];
} uixo_message_t;

typedef struct {
	struct list_head list;
	struct list_head msghead;
    pthread_t rx_msg_thread;
    int rx_thread_is_run;
    pthread_mutex_t port_mutex;

    /* uixo message head */
    char* rx_head;
    char* tx_head;
    /* uixo data source port */
    void* port;
    char* baudrate;
    char* bytesize;
    char* parity;
    char* stopbits;
    char* timeout;
    char* name;
    /* uixo files saved cmd */
    int   rx_cmd_fd;
    int   tx_cmd_fd;
    off_t rx_cmd_off;
    off_t tx_cmd_off;
} uixo_port_t;

#if 0
enum uixo_rx_status;
typedef enum {
	UIXO_RX_STATUS_IDLE = 0,
	UIXO_RX_STATUS_GOT_INVALID,
	UIXO_RX_STATUS_GOT_HEAD,
	UIXO_RX_STATUS_GOT_MSG
}uixo_rx_status;
#endif


/*
    global functions
*/
extern long calloc_count;
static inline void* uixo_console_calloc(size_t count, size_t size)
{
    void* ptr = NULL;
    ptr = calloc(count, size);
    if(NULL == ptr) {
        printf("%s: calloc error.\n", __func__);
        return NULL;
    }
    calloc_count++;
    //PR_DEBUG("%s: calloc mem addr=0x%08x, len=%d calloc_count=%d.\n", __func__, (int)ptr, count*size, calloc_count);
    return ptr;
}

static inline void uixo_console_free(void* ptr)
{
    if(NULL != ptr) {
        free(ptr);
        calloc_count--;
        //PR_DEBUG("%s: free mem addr=0x%08x, calloc_count=%d.\n", __func__, (int)ptr, calloc_count);
    }
    else {
        printf("%s: free error.\n", __func__);
    }
}

uixo_port_t* handle_port_mkport(const char* port_name, const int baudrate);
int handle_port_delport(const char* port_name);
int handle_port_hlport(uixo_message_t* msg);
void handle_port_remove_port_list(void);
int handle_port_read_line(uixo_port_t* port, char* rx_data, const int len);
int handle_port_fun_types(uixo_message_t* msg);
int handle_msg_del_msg(uixo_message_t* msg);
int handle_msg_del_msglist(struct list_head* msg_head);
int handle_msg_transmit_data(uixo_port_t* port, uixo_message_t* msg);
int handle_msg_receive_data(uixo_port_t* port);
int handle_msg_resolve_msg(const int fd);

#endif
