/*
    FileName    :uARM_driver.h
    Description :head file of uARM_driver
    Author      :SchumyHao
    Email       :schumy.haojl@gmail.com
    Version     :V10
    Data        :2014.04.07
*/
#ifndef __UARM_DRIVER_H__
#define __UARM_DRIVER_H__

/* Include files */
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

/* PI define */
#define PI                     (3.1415926)
/* Arm moving path define */
#define GO_WITH_LINE
//#define GO_WITH_PARABOLA
/* Max pick up retry times */
#define PICK_RETRY_TIMES       (20)
/* Coin's thickness */
#define COIN_THICKNESS         (10)
/* Frame delay time */
#define FRAME_DELAY_TIME       (2000)
/* Parameter offset */
#define OFFSET                 (32768)
/* Frame head */
#define FRAME_HEADER_H         ((unsigned char) 0xFF)
#define FRAME_HEADER_L         ((unsigned char) 0xAA)
/* Buff define */
#define BUFFER_SIZE            (7)
#define BUFFER_DEEP            (2000)
#define DEFAULT_BUFF_DEEP      (0)
/* uArm's location define */
#define UARM_X_LOCATION        (0)
#define UARM_Y_LOCATION        (-94)
/* Coin destination define */
#define DEFAULT_DEST           (0)
#define DEST_ONE               (-1)
#define DEST_ONE_A             (-180)
#define DEST_ONE_R             (80)
#define DEST_ONE_H             (100)
#define DEST_FIVE              (1)
#define DEST_FIVE_A            (180)
#define DEST_FIVE_R            (80)
#define DEST_FIVE_H            (100)
#define DEST_USER              (2)
#define DEST_USER_A            (0)
#define DEST_USER_R            (200)
#define DEST_USER_H            (100)
#define IS_DESTINATION(DEST)   (((DEST) == DEST_ONE) || \
                                ((DEST) == DEST_FIVE) || \
                                ((DEST) == DEST_USER) || \
                                ((DEST) == DEFAULT_DEST))
/* uArm motion define */
#define MOTION_NONE            (0x20)
#define MOTION_H_UP            (0x80)
#define MOTION_H_DOWN          (0x40)
#define MOTION_SHIFT           (0x10)
#define MOTION_GROUND          (0x04)
#define MOTION_RELEASE         (0x02)
#define MOTION_PICK            (0x01)
#define IS_MOTION(MOTION)      (((MOTION) == MOTION_H_UP) || \
                                ((MOTION) == MOTION_H_DOWN) || \
                                ((MOTION) == MOTION_SHIFT) || \
                                ((MOTION) == MOTION_GROUND) || \
                                ((MOTION) == MOTION_RELEASE) || \
                                ((MOTION) == MOTION_PICK) || \
                                ((MOTION) == MOTION_NONE))

typedef enum {
    DISABLE = 0,
    ENABLE = !DISABLE
} FunctionalState;
#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == DISABLE) || ((STATE) == ENABLE))

typedef struct {
    FunctionalState CooShiftEn;
    FunctionalState DirectOutputEn;
    int X;
    int Y;
    int H;
    int Angle;
    int Radius;
    int Dest;
} t_Coordinate;
/* X coordinate */
#define DEFAULT_X_LOCATION  (0)
#define MAX_X_LOCATION      (150)
#define MIN_X_LOCATION      (-150)
#define IS_X_LOCATION(X)    (((X) >= MIN_X_LOCATION) && \
                             ((X) <= MAX_X_LOCATION))
/* Y coordinate */
#define DEFAULT_Y_LOCATION  (0)
#define MAX_Y_LOCATION      (200)
#define MIN_Y_LOCATION      (0)
#define IS_Y_LOCATION(Y)    (((Y) >= MIN_Y_LOCATION) && \
                             ((Y) <= MAX_Y_LOCATION))
/* H(Hight) coordinate*/
#define DEFAULT_H_LOCATION  (100)
#define MAX_H_LOCATION      (100)
#define MIN_H_LOCATION      (-110)
#define IS_H_LOCATION(H)    (((H) >= MIN_H_LOCATION) && \
                             ((H) <= MAX_H_LOCATION))
/* A(Angle) coordinate */
#define DEFAULT_A_DEGREE    (0)
#define MAX_A_DEGREE        (180)
#define MIN_A_DEGREE        (-180)
#define IS_A_DEGREE(A)      (((A) >= MIN_A_DEGREE) && \
                             ((A) <= MAX_A_DEGREE))
/* R(Radius) coordinate */
#define DEFAULT_R_LENGTH    (0)
#define MAX_R_LENGTH        (200)
#define MIN_R_LENGTH        (0)
#define IS_R_LENGTH(R)      (((R) >= MIN_R_LENGTH) && \
                             ((R) <= MAX_R_LENGTH))
typedef struct {
    int DestAngle;
    int DestRadius;
    int DestHight;
    int CurrAngle;
    int CurrRadius;
    int CurrHight;
} t_Move;



/* Macro function */
#define HI_BYTE(x)      ((unsigned char)((((x)+OFFSET) & 0xff00) >> 8))
#define LO_BYTE(x)      ((unsigned char)(((x)+OFFSET) & 0x00ff))
#define RAD2ANG(RAD)        ((RAD)*180/(PI))
#define SINGNAL(X)      (((X) > 0)?1:(((X) == 0)?0:-1))
#define MAX2(X,Y)       (((X) >= (Y))?(X):(Y))

/* Function declaration */
/* Control data transmit function */
int SendData(FILE* const pFp, int const BuffDeep, const char* pBuff);
/* Coordinate initialize function */
int InitCoordinateSystem(t_Coordinate* pCooSys);
/* Change the coordinate from rectangular to polar */
int ShiftCoordinate(t_Coordinate* pCooSys);
/* Motion data Generation function */
int GenerateMotion(t_Coordinate* pCooSys, char* pBuff);
/* Arm move function */
int MoveArm(t_Move* pMotion, char* pBuff, int BuffDeep);
/* Arm handle function */
int HandleArm(const unsigned char Motion, char* pBuff, int BuffDeep);
#endif
