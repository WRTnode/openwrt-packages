/*
	文件：uARM_driver.h
	说明：uARM驱动头文件
	作者：SchumyHao
	版本：V01
	日期：2013.03.19
*/
#ifndef __UARM_DRIVER_H__
#define __UARM_DRIVER_H__

/* 头文件 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>

/* 参数定义 */
/* PI定义 */
#define PI			(3.1415926)
/* 机械臂移动路径选择 */
#define GO_WITH_LINE
//#define GO_WITH_PARABOLA
/* 吸取硬币尝试最大次数 */
#define PICK_RETRY_TIMES	(5)
/* 数据帧间隔时间 */
#define FRAME_DELAY_TIME	(100000)
/* 参数调节偏移量 */
#define OFFSET			(32768)
/* 数据帧头 */
#define FRAME_HEADER_H		((unsigned char) 0xFF)
#define FRAME_HEADER_L		((unsigned char) 0xAA)
/* 发送指令缓存长度和深度 */
#define BUFFER_SIZE 		(7)
#define BUFFER_DEEP 		(1000)
#define DEFAULT_BUFF_DEEP	(0)
typedef struct{
	int X;
	int Y;
	int Angle;
	int Radius;
	int DestAngle;
	int DestRadius;
	int CurrAngle;
	int CurrRadius;
}t_coordinate;
/* X坐标定义 */
#define DEFAULT_X_LOCATION	(0)
#define MAX_X_LOCATION		(100)
#define MIN_X_LOCATION		(-100)
#define IS_X_LOCATION(X)	(((X) >= MIN_X_LOCATION) && \
				((X) <= MAX_X_LOCATION))
/* Y坐标定义 */
#define DEFAULT_Y_LOCATION	(0)
#define MAX_Y_LOCATION		(50)
#define MIN_Y_LOCATION		(0)
#define IS_Y_LOCATION(Y)	(((Y) >= MIN_Y_LOCATION) && \
				((Y) <= MAX_Y_LOCATION))
/* H坐标定义 */
#define MAX_H_POSITION		(100)
#define MIN_H_POSITION		(0)
/* 机械臂坐标定义 */
#define UARM_X_LOCATION		(0)
#define UARM_Y_LOCATION		(-10)
/* 目的地址定义 */
#define DEFAULT_DEST		(0)
#define DEFAULT_DEST_A		(0)
#define DEFAULT_DEST_R		(0)
#define DEST_ONE		(-1)
#define DEST_ONE_A		(-180)
#define DEST_ONE_R		(50)
#define DEST_FIVE		(1)
#define DEST_FIVE_A		(180)
#define DEST_FIVE_R		(50)
#define IS_DESTINATION(DEST)	(((DEST) == DEST_ONE) || \
				((DEST) == DEST_FIVE) || \
				((DEST) == DEFAULT_DEST))
#define IS_DESTINATION_A(DEST)	(((DEST) == DEST_ONE_A) || \
				((DEST) == DEST_FIVE_A) || \
				((DEST) == DEFAULT_DEST_A))
#define IS_DESTINATION_R(DEST)	(((DEST) == DEST_ONE_R) || \
				((DEST) == DEST_FIVE_R) || \
				((DEST) == DEFAULT_DEST_R))
/* A(Angle)坐标定义 */
#define DEFAULT_A_DEGREE	(0)
#define MAX_A_DEGREE		(180)
#define MIN_A_DEGREE		(-180)
#define IS_A_DEGREE(A)		(((A) >= MIN_A_DEGREE) && \
				((A) <= MAX_A_DEGREE))
/* R(Radius)坐标定义 */
#define DEFAULT_R_LENGTH	(0)
#define MAX_R_LENGTH		(200)
#define MIN_R_LENGTH		(0)
#define IS_R_LENGTH(R)		(((R) >= MIN_R_LENGTH) && \
				((R) <= MAX_R_LENGTH))
/* 动作定义 */
#define MOTION_NONE		(0x20)
#define MOTION_H_UP		(0x80)
#define MOTION_H_DOWN		(0x40)
#define MOTION_SHIFT		(0x10)
#define MOTION_GROUND		(0x04)
#define MOTION_RELEASE		(0x02)
#define MOTION_PICK		(0x01)
#define IS_MOTION(MOTION)	(((MOTION) == MOTION_H_UP) || \
				((MOTION) == MOTION_H_DOWN) || \
				((MOTION) == MOTION_SHIFT) || \
				((MOTION) == MOTION_GROUND) || \
				((MOTION) == MOTION_RELEASE) || \
				((MOTION) == MOTION_PICK) || \
				((MOTION) == MOTION_NONE))
/* 宏定义函数 */
#define HI_BYTE(x) 		((unsigned char)((((x)+OFFSET) & 0xff00) >> 8))
#define LO_BYTE(x) 		((unsigned char)(((x)+OFFSET) & 0x00ff))
#define RAD2ANG(RAD)		((RAD)*180/(PI))
#define SINGNAL(X)		(((X) > 0)?1:(((X) == 0)?0:-1))
#define MAX2(X,Y)		(((X) >= (Y))?(X):(Y))

/* 函数声明 */
/* 发送控制指令函数 */
//TODO:修改新的形式
int SendData(FILE* const pFp, int const BuffDeep, const char* pBuff);
/* 坐标系结构体参数初始化 */
int InitCoordinateSystem(t_coordinate* pCooSys);
/* 坐标系参数变换 */
int ShiftCoordinate(t_coordinate* pCooSys);
/* 动作生成 */
int GenerateMotion(t_coordinate* pCooSys, char *pBuff);
#endif
