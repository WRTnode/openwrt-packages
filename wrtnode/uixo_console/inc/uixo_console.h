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

/*
    const define
*/
#define MAX_UIXO_MSG_LEN       (300)
/*
    typedef of uixo
*/
typedef enum {
    UIXO_CONSOLE_ERR_INPUT_NULL = 1,
    UIXO_CONSOLE_ERR_READ_PORT,
    UIXO_CONSOLE_ERR_WRITE_PORT,
    UIXO_CONSOLE_ERR_SAVE_CMD,
    UIXO_CONSOLE_ERR_LOAD_CMD,
    UIXO_CONSOLE_ERR_PROC_MSG,
    UIXO_CONSOLE_ERR_STATUS,
    UIXO_CONSOLE_ERR_MEM
}uixo_error_t;

typedef struct {
    /* uixo message head */
    char* rx_head;
    char* tx_head;
    /* uixo data source port */
    void* port;
    /* uixo files saved cmd */
    int   rx_cmd_fd;
    int   tx_cmd_fd;
    off_t rx_cmd_off;
    off_t tx_cmd_off;
} uixo_port_t;
typedef struct {
    unsigned long time;
    unsigned char len;
    char          cmd;
    char*         data;
} uixo_message_t;
/*
    global functions
*/
int uixo_rx_handler(uixo_port_t* p);
int uixo_tx_handler(uixo_port_t* p);
#endif
