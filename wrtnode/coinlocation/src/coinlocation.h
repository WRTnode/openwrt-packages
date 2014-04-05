/*
	FileName	£ºcoinlocation.h
	Description	£ºhead file of coinlocation
	Author		£ºSchumyHao
	Version		£ºV01
	Data		£º2013.03.30
*/
#ifndef __COINLOCATION_H__
#define __COINLOCATION_H__

/* Include files */
#include <stdio.h>
#include <assert.h>

//TODO:Now, only can input rectengular working area.

/* Bottom left  define */
#define BOTTOM_LEFT_UARM_X			(-72)
#define BOTTOM_LEFT_UARM_Y			(15)
#define BOTTOM_LEFT_MIN_PIXEL_Y		(0)
#define BOTTOM_LEFT_MIN_PIXEL_X		(0)
#define BOTTOM_LEFT_MAX_PIXEL_Y		(100)
#define BOTTOM_LEFT_MAX_PIXEL_X		(100)
#define IS_BOTTOM_LEFT_PIXEL_X(X)		(((X) <= BOTTOM_LEFT_MAX_PIXEL_X) && \
										((X) >= BOTTOM_LEFT_MIN_PIXEL_X))
#define IS_BOTTOM_LEFT_PIXEL_Y(Y)		(((Y) <= BOTTOM_LEFT_MAX_PIXEL_Y) && \
										((Y) >= BOTTOM_LEFT_MIN_PIXEL_Y))
/* Top left define */
#define TOP_LEFT_UARM_X			(-72)
#define TOP_LEFT_UARM_Y			(181)
#define TOP_LEFT_MIN_PIXEL_Y		(540)
#define TOP_LEFT_MIN_PIXEL_X		(0)
#define TOP_LEFT_MAX_PIXEL_Y		(640)
#define TOP_LEFT_MAX_PIXEL_X		(100)
#define IS_TOP_LEFT_PIXEL_X(X)		(((X) <= TOP_LEFT_MAX_PIXEL_X) && \
									((X) >= TOP_LEFT_MIN_PIXEL_X))
#define IS_TOP_LEFT_PIXEL_Y(Y)		(((Y) <= TOP_LEFT_MAX_PIXEL_Y) && \
									((Y) >= TOP_LEFT_MIN_PIXEL_Y))
/* Bottom right  define */
#define BOTTOM_RIGHT_UARM_X			(52)
#define BOTTOM_RIGHT_UARM_Y			(15)
#define BOTTOM_RIGHT_MIN_PIXEL_Y		(0)
#define BOTTOM_RIGHT_MIN_PIXEL_X		(380)
#define BOTTOM_RIGHT_MAX_PIXEL_Y		(100)
#define BOTTOM_RIGHT_MAX_PIXEL_X		(480)
#define IS_BOTTOM_RIGHT_PIXEL_X(X)		(((X) <= BOTTOM_RIGHT_MAX_PIXEL_X) && \
										((X) >= BOTTOM_RIGHT_MIN_PIXEL_X))
#define IS_BOTTOM_RIGHT_PIXEL_Y(Y)		(((Y) <= BOTTOM_RIGHT_MAX_PIXEL_Y) && \
										((Y) >= BOTTOM_RIGHT_MIN_PIXEL_Y))
/* Top left define */
#define TOP_RIGHT_UARM_X			(52)
#define TOP_RIGHT_UARM_Y			(181)
#define TOP_RIGHT_MIN_PIXEL_Y		(540)
#define TOP_RIGHT_MIN_PIXEL_X		(380)
#define TOP_RIGHT_MAX_PIXEL_Y		(640)
#define TOP_RIGHT_MAX_PIXEL_X		(480)
#define IS_TOP_RIGHT_PIXEL_X(X)		(((X) <= TOP_RIGHT_MAX_PIXEL_X) && \
									((X) >= TOP_RIGHT_MIN_PIXEL_X))
#define IS_TOP_RIGHT_PIXEL_Y(Y)		(((Y) <= TOP_RIGHT_MAX_PIXEL_Y) && \
									((Y) >= TOP_RIGHT_MIN_PIXEL_Y))
typedef struct{
	int UarmX;
	int UarmY;
	int PixelX;
	int PixelY;
}t_Point;
#define MAX_UARM_X				(100)
#define MIN_UARM_X				(-100)
#define MAX_UARM_Y				(190)
#define MIN_UARM_Y				(0)
#define IS_UARM_X(X)			(((X) <= MAX_UARM_X) && \
								((X) >= MIN_UARM_X))
#define IS_UARM_Y(Y)			(((Y) <= MAX_UARM_Y) && \
								((Y) >= MIN_UARM_Y))
#define IS_UARM_POINT(X,Y)		(((X) <= MAX_UARM_X) && \
								((X) >= MIN_UARM_X) && \
								((Y) <= MAX_UARM_Y) && \
								((Y) >= MIN_UARM_Y))
#define MAX_PIXEL_X				(480)
#define MIN_PIXEL_X				(0)
#define MAX_PIXEL_Y				(640)
#define MIN_PIXEL_Y				(0)
#define IS_PIXEL_X(X)			(((X) <= MAX_PIXEL_X) && \
								((X) >= MIN_PIXEL_X))
#define IS_PIXEL_Y(Y)			(((Y) <= MAX_PIXEL_Y) && \
								((Y) >= MIN_PIXEL_Y))
#define IS_PIXEL_POINT(X,Y)		(((X) <= MAX_PIXEL_X) && \
								((X) >= MIN_PIXEL_X) && \
								((Y) <= MAX_PIXEL_Y) && \
								((Y) >= MIN_PIXEL_Y))

typedef struct{
	int PointNumber;
	t_Point* pPoint;
	t_Point StartPoint;
	t_Point EndPoint;
}t_Line;

typedef struct{
	int LineNumber;
	t_Line* pLine;
	t_Point BottomLeftPoint;
	t_Point BottomRightPoint;
	t_Point TopLeftPoint;
	t_Point TopRightPoint;
}t_Plane;


/* Macro function */

/* Function declaration */
/* Draw point */
int PointDraw(t_Point* pPoint, int const UarmX, int const UarmY, int const PixelX, int const PixelY);
/* Print a point */
void PointPrint(char* pPointName, t_Point* pPoint);
/* Draw horizon line */
int HorizonLineDraw(t_Line* pLine);
/* Draw rectengular area */
int RectengularAreaDraw(t_Plane* pArea);
/* Coin location. Input coin's pixel, output Coin point */
int CoinLocation(t_Point* pCoin, t_Plane* pWorkingArea);
/* Copy a point. */
int CopyPoint(t_Point* pPointdst, const t_Point* pPointsrc);
/* Copy a line */
int CopyLine(t_Line* pLinedst, const t_Line* pLinesrc);
/* Write points map */
int WritePointsMap(t_Plane* pPlane);
#endif
