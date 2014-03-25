/*
	文件：UART.h
	说明：串口控制函数头文件
	作者：SchumyHao
	版本：V02
	日期：2013.03.18
*/

#ifndef __UART_H__
#define __UART_H__

/* 头文件 */
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

/* 结构体定义 */
typedef struct{
	int Fd;
	FILE* pFp;
	int BaudRate;
	int DataBits;
	int StopBits;
	int Parity;
}t_uart;
/* 文件标识符定义 */
#define INIT_FD			(255)
#define IS_FD(FD)		((FD) >= 0)
/* 波特率定义 */
#define BAUD_RATE_2400		(1)
#define BAUD_RATE_4800		(2)
#define BAUD_RATE_9600		(3)
#define BAUD_RATE_19200		(4)
#define BAUD_RATE_38400		(5)
#define BAUD_RATE_57600		(6)
#define BAUD_RATE_115200	(7)
#define IS_BAUD_RATE(BAUDRATE)	(((BAUDRATE) == BAUD_RATE_2400) || \
				((BAUDRATE) == BAUD_RATE_4800) || \
				((BAUDRATE) == BAUD_RATE_9600) || \
				((BAUDRATE) == BAUD_RATE_19200) || \
				((BAUDRATE) == BAUD_RATE_38400) || \
				((BAUDRATE) == BAUD_RATE_57600) || \
				((BAUDRATE) == BAUD_RATE_115200))
/* 数据位定义 */
#define DATA_BITS_7BITS		(1)
#define DATA_BITS_8BITS		(2)
#define IS_DATA_BITS(DATABITS)	(((DATABITS) == DATA_BITS_7BITS) || \
				((DATABITS) == DATA_BITS_8BITS))
/* 停止位定义 */
#define STOP_BITS_1BIT		(1)
#define STOP_BITS_2BITS		(2)
#define IS_STOP_BITS(STOPBITS)	(((STOPBITS) == STOP_BITS_1BIT) || \
				((STOPBITS) == STOP_BITS_2BITS))
/* 校验位定义 */
#define PARITY_O		(1)
#define PARITY_E		(2)
#define PARITY_NONE		(3)
#define IS_PARITY(PARITY)	(((PARITY) == PARITY_O) || \
				((PARITY) == PARITY_E) || \
				((PARITY) == PARITY_NONE))

/* 串口端口定义 */
#define MAX_COM_PORT_NUM	(3)
#define MIN_COM_PORT_NUM	(0)
#define IS_COM_PORT(PORT)	(((PORT) >= MIN_COM_PORT_NUM) && \
				((PORT) <= MAX_COM_PORT_NUM))

/* 函数声明 */
/* 串口配置函数 */
int ConfigUart(t_uart* pUart, struct termios* pOldCfg);
/* 串口结构体初始化函数 */
int InitUartStruct(t_uart* pUart);
/* 打开串口函数 */
int OpenPort(int const ComPort);

#endif
