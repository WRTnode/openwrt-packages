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
#include "list.h"

/*
    const define
*/
#define MAX_UIXO_MSG_LEN       (300)
/*

*/
#define DugPrintg  1
#if DugPrintg 
#define PR_DEBUG(fmt,args...) printf(fmt,##args)
#else
#define PR_DEBUG(fmt,args...) /*do nothing */
#endif

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
	unsigned long   time;
	unsigned char   len;
	int 			timeout;
	int 			socketfd;
	int 			rttimes;
	int			    currenttime;
	char            cmd;
	char*           data;
	char* 			port_name;
	int 			port_baudrate;
    char*           fn_name;
} uixo_message_t;

typedef struct {
	struct list_head list;
	struct list_head* msghead;
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
enum uixo_rx_status;
#if 0
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
int uixo_rx_handler(uixo_port_t* p,char* Callback);
int del_msg(uixo_port_t* p,struct list_head* pos,struct list_head* n,char* msg_name,int socketfd,int flag);
int FunTypes(struct list_head* list,uixo_message_t* onemsg,char* fn_name);
int mkport(uixo_port_t* port,char* port_name,char* baudrate,struct list_head* list);
uixo_err_t uixo_port_open(uixo_port_t* port);
int del_port(struct list_head* pos,struct list_head* n,char* port_name,struct list_head* list);
int del_msglist(uixo_port_t* p,struct list_head* pos,struct list_head* n);
int uixo_transmit_data(uixo_port_t* p, const uixo_message_t* msg);
int uixo_receive_data(uixo_port_t* p, uixo_message_t** msg);
int uixo_receive_data_process(const char ch, uixo_message_t** msg,enum uixo_rx_status* status, char* head);
int uixo_save_cmd(uixo_port_t* p, const uixo_message_t* msg,char* Callback);
int uixo_invalid_receive_data_process(void* port, char* str, int size);
int uixo_console_parse_msg(const char* data, const ssize_t len, uixo_message_t* msg)

#endif
