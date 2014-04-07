/*
    FileName    :UART.h
    Description :head file of UART config.
    Author      :SchumyHao
    Email       :schumy.haojl@gmail.com
    Version     :V10
    Data        :2014.04.07
*/

#ifndef __UART_H__
#define __UART_H__

/* Include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

/* Structure define */
typedef struct {
    FILE* pFp;
    int Fd;
    int BaudRate;
    int DataBits;
    int StopBits;
    int Parity;
} t_uart;
/* FD define */
#define INIT_FD             (255)
#define IS_FD(FD)           ((FD) >= 0)
/* Baudrate define */
#define BAUD_RATE_2400      (1)
#define BAUD_RATE_4800      (2)
#define BAUD_RATE_9600      (3)
#define BAUD_RATE_19200     (4)
#define BAUD_RATE_38400     (5)
#define BAUD_RATE_57600     (6)
#define BAUD_RATE_115200    (7)
#define IS_BAUD_RATE(BAUDRATE)  (((BAUDRATE) == BAUD_RATE_2400) || \
                                 ((BAUDRATE) == BAUD_RATE_4800) || \
                                 ((BAUDRATE) == BAUD_RATE_9600) || \
                                 ((BAUDRATE) == BAUD_RATE_19200) || \
                                 ((BAUDRATE) == BAUD_RATE_38400) || \
                                 ((BAUDRATE) == BAUD_RATE_57600) || \
                                 ((BAUDRATE) == BAUD_RATE_115200))
/* Databits define */
#define DATA_BITS_7BITS     (1)
#define DATA_BITS_8BITS     (2)
#define IS_DATA_BITS(DATABITS)  (((DATABITS) == DATA_BITS_7BITS) || \
                                 ((DATABITS) == DATA_BITS_8BITS))
/* Stopbits define */
#define STOP_BITS_1BIT      (1)
#define STOP_BITS_2BITS     (2)
#define IS_STOP_BITS(STOPBITS)  (((STOPBITS) == STOP_BITS_1BIT) || \
                                 ((STOPBITS) == STOP_BITS_2BITS))
/* Parity define */
#define PARITY_O            (1)
#define PARITY_E            (2)
#define PARITY_NONE         (3)
#define IS_PARITY(PARITY)   (((PARITY) == PARITY_O) || \
                             ((PARITY) == PARITY_E) || \
                             ((PARITY) == PARITY_NONE))

/* UART port number define */
#define MAX_COM_PORT_NUM    (3)
#define MIN_COM_PORT_NUM    (0)
#define IS_COM_PORT(PORT)   (((PORT) >= MIN_COM_PORT_NUM) && \
                             ((PORT) <= MAX_COM_PORT_NUM))

/* Function declaration */
/* Congit UART function */
int ConfigUart(t_uart* pUart, struct termios* pOldCfg);
/* UART initialize function */
int InitUartStruct(t_uart* pUart);
/* Open UART's device file function */
int OpenPort(int const ComPort);

#endif
