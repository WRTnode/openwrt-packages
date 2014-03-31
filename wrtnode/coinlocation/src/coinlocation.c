/*
	FileName	£ºcoinlocation.c
	Description	£ºSource code of coin location.
	Author		£ºSchumyHao
	Version		£ºV01
	Data		£º2013.03.30
*/

/* Uncomment next line if you want to debug the code. */
//#define DEBUG

/* Include files */
#include "coinlocation.h"

/* Functions */
/* Draw point */
int PointDraw(t_Point* pPoint, int const UarmX, int const UarmY, int const PixelX, int const PixelY){
	assert(IS_PIXEL_X(PixelX));
	assert(IS_PIXEL_Y(PixelY));
	assert(IS_UARM_X(UarmX));
	assert(IS_UARM_Y(UarmY));

	pPoint->PixelX = PixelX;
	pPoint->PixelY = PixelY;
	pPoint->UarmX = UarmX;
	pPoint->UarmY = UarmY;
	return 0;
}
/* Print a point */
void PointPrint(char* pPointName, t_Point* pPoint){
	printf("%s in uArm (%3d,%3d) in PixelX (%3d,%3d)\n", \
		pPointName, pPoint->UarmX,pPoint->UarmY,pPoint->PixelX,pPoint->PixelY);
}

/* Draw horizon line */
int HorizonLineDraw(t_Line* pLine){
	t_Point* pPoint = pLine->pPoint;
	int dUarmX = pLine->EndPoint.UarmX - pLine->StartPoint.UarmX;
	float dPixelX = (float)(pLine->EndPoint.PixelX-pLine->StartPoint.PixelX)/dUarmX;
	float dPixelY = (float)(pLine->EndPoint.PixelY-pLine->StartPoint.PixelY)/dUarmX;
	if(pLine->StartPoint.UarmY == pLine->EndPoint.UarmY){
		PointDraw(pPoint, pLine->StartPoint.UarmX, \
				pLine->StartPoint.UarmY, \
				pLine->StartPoint.PixelX, \
				pLine->StartPoint.PixelY);
		pLine->PointNumber++;
		while(pLine->EndPoint.UarmX > pPoint->UarmX){
			pPoint ++;
			CopyPoint(pPoint, pPoint-1);
			pPoint->PixelX = (int)(pLine->StartPoint.PixelX + \
						dPixelX * pLine->PointNumber);
			pPoint->PixelY = (int)(pLine->StartPoint.PixelY + \
						dPixelY * pLine->PointNumber);
			pPoint->UarmX++;
			pLine->PointNumber++;
		}
		#ifdef DEBUG
		PointPrint("Start point", &(pLine->StartPoint));
		PointPrint("End point", &(pLine->EndPoint));
		printf("Point number = %d\n",pLine->PointNumber);
		#endif
		return 0;
	}
	else{
		printf("Can not draw a horizon line.\n");
		return -1;
	}
}
/* Draw rectengular area */
int RectengularAreaDraw(t_Plane* pArea){
	t_Line* pLine = pArea->pLine;
	int dUarmLeftY = pArea->TopLeftPoint.UarmY - pArea->BottomLeftPoint.UarmY;
	int dUarmRightY = pArea->TopRightPoint.UarmY - pArea->BottomRightPoint.UarmY;
	float dPixelLeftX = (float)(pArea->TopLeftPoint.PixelX-pArea->BottomLeftPoint.PixelX)/dUarmLeftY;
	float dPixelLeftY = (float)(pArea->TopLeftPoint.PixelY-pArea->BottomLeftPoint.PixelY)/dUarmLeftY;
	float dPixelRightX = (float)(pArea->TopRightPoint.PixelX-pArea->BottomRightPoint.PixelX)/dUarmRightY;
	float dPixelRightY = (float)(pArea->TopRightPoint.PixelY-pArea->BottomRightPoint.PixelY)/dUarmRightY;
	/*
	t_Point HighestPoint;
	t_Point LowestPoint;
	t_Point LeftMostPoint;
	t_Point RightMostPoint;
	*/

	/* Find Lowest point. */
	/*if(pRefPoint->BottomLeftPoint.UarmY <= pRefPoint->BottomRightPoint.UarmY){
		PointDraw(&LowestPoint, \
			pRefPoint->BottomLeftPoint.UarmX, \
			pRefPoint->BottomLeftPoint.UarmY, \
			pRefPoint->BottomLeftPoint.PixelX, \
			pRefPoint->BottomLeftPoint.PixelY)
	}
	else{
		PointDraw(&LowestPoint, \
			pRefPoint->BottomRightPoint.UarmX, \
			pRefPoint->BottomRightPoint.UarmY, \
			pRefPoint->BottomRightPoint.PixelX, \
			pRefPoint->BottomRightPoint.PixelY)
	}*/
	/* Find Highest point. */
	/*if(pRefPoint->TopLeftPoint.UarmY >= pRefPoint->TopRightPoint.UarmY){
		PointDraw(&HighestPoint, \
			pRefPoint->TopLeftPoint.UarmX, \
			pRefPoint->TopLeftPoint.UarmY, \
			pRefPoint->TopLeftPoint.PixelX, \
			pRefPoint->TopLeftPoint.PixelY)
	}
	else{
		PointDraw(&HighestPoint, \
			pRefPoint->TopRightPoint.UarmX, \
			pRefPoint->TopRightPoint.UarmY, \
			pRefPoint->TopRightPoint.PixelX, \
			pRefPoint->TopRightPoint.PixelY)
	}*/
	/* Find left most point. */
	/*if(pRefPoint->TopLeftPoint.UarmX < pRefPoint->BottomLeftPoint.UarmX){
		PointDraw(&LeftMostPoint, \
			pRefPoint->TopLeftPoint.UarmX, \
			pRefPoint->TopLeftPoint.UarmY, \
			pRefPoint->TopLeftPoint.PixelX, \
			pRefPoint->TopLeftPoint.PixelY)
	}
	else{
		PointDraw(&LeftMostPoint, \
			pRefPoint->BottomLeftPoint.UarmX, \
			pRefPoint->BottomLeftPointUarmY, \
			pRefPoint->BottomLeftPoint.PixelX, \
			pRefPoint->BottomLeftPoint.PixelY)
	}*/
	/* Find right most point */
	/*if(pRefPoint->TopRightPoint.UarmX > pRefPoint->BottomRightPoint.UarmX){
		PointDraw(&RightMostPoint, \
			pRefPoint->TopRightPoint.UarmX, \
			pRefPoint->TopRightPoint.UarmY, \
			pRefPoint->TopRightPoint.PixelX, \
			pRefPoint->TopRightPoint.PixelY)
	}
	else{
		PointDraw(&RightMostPoint, \
			pRefPoint->BottomRightPoint.UarmX, \
			pRefPoint->BottomRightPointUarmY, \
			pRefPoint->BottomRightPoint.PixelX, \
			pRefPoint->BottomRightPoint.PixelY)
	}*/

	/* First line start point is the lowest point */
	PointDraw(&(pLine->StartPoint), \
		pArea->BottomLeftPoint.UarmX, \
		pArea->BottomLeftPoint.UarmY, \
		pArea->BottomLeftPoint.PixelX, \
		pArea->BottomLeftPoint.PixelY);
	/* First line End point */
	PointDraw(&(pLine->EndPoint), \
		pArea->BottomRightPoint.UarmX, \
		pArea->BottomRightPoint.UarmY, \
		pArea->BottomRightPoint.PixelX, \
		pArea->BottomRightPoint.PixelY);

	HorizonLineDraw(pLine);
	pArea->LineNumber++;
	while((pLine->StartPoint.UarmY < pArea->TopLeftPoint.UarmY) && \
		(pLine->EndPoint.UarmY < pArea->TopRightPoint.UarmY)){
		pLine++;
		CopyLine(pLine, pLine-1);
		/* Update next line's start point. */
		pLine->StartPoint.PixelX = (int)(pArea->BottomLeftPoint.PixelX + \
						dPixelLeftX * pArea->LineNumber);
		pLine->StartPoint.PixelY = (int)(pArea->BottomLeftPoint.PixelY + \
						dPixelLeftY * pArea->LineNumber);
		pLine->StartPoint.UarmY++;
		/* Update next line's end point. */
		pLine->EndPoint.PixelX = (int)(pArea->BottomRightPoint.PixelX + \
						dPixelRightX * pArea->LineNumber);
		pLine->EndPoint.PixelY = (int)(pArea->BottomRightPoint.PixelY + \
						dPixelRightY * pArea->LineNumber);
		pLine->EndPoint.UarmY++;
		/* Update next line's pPoint. */
		pLine->pPoint += pLine->PointNumber;
		/* Update next line's points number. */
		pLine->PointNumber = 0;
		HorizonLineDraw(pLine);
		pArea->LineNumber++;
	}
	return 0;
}
/* Coin location. Input coin's pixel, output Coin point */
int CoinLocation(t_Point* pCoin, t_Plane* pWorkingArea){
	assert(IS_PIXEL_X(pCoin->PixelX));
	assert(IS_PIXEL_Y(pCoin->PixelY));
	t_Line* pLine = pWorkingArea->pLine;
	t_Point* pPoint = pWorkingArea->pLine->pPoint;
	int MaxLineNum = pWorkingArea->LineNumber;
	int MaxPointNum = pLine->PointNumber;
	int CoinPixelX = pCoin->PixelX;
	int CoinPixelY = pCoin->PixelY;
	int Line;
	int Row;

	for(Line=0; Line < MaxLineNum; Line++, pLine++){
		/*#ifdef DEBUG
		printf("CoinPixelY=%d\n",CoinPixelY);
		printf("pLine->StartPoint.PixelY=%d\n",pLine->StartPoint.PixelY);
		#endif*/
		if(CoinPixelY <= pLine->StartPoint.PixelY)
			break;
	}
	if(Line < MaxLineNum){
		pPoint = pLine->pPoint;
		MaxPointNum = pLine->PointNumber;
		for(Row=0; Row < MaxPointNum; Row++, pPoint++){
			/*#ifdef DEBUG
			printf("CoinPixelX=%d\n",CoinPixelX);
			printf("pPoint->PixelX=%d\n",pPoint->PixelX);
			#endif*/
			if(CoinPixelX <= pPoint->PixelX)
				break;
		}
		if(Row < MaxPointNum){
			pCoin->UarmX = pPoint->UarmX;
			pCoin->UarmY = pPoint->UarmY;
			return 0;
		}
		else{
			printf("Coin is out of boundry!\n");
			return -1;
		}
	}
	else{
		printf("Coin is out of boundry!\n");
		return -2;
	}
}
/* Copy a point. */
int CopyPoint(t_Point* pPointdst, const t_Point* pPointsrc){
	pPointdst->PixelX = pPointsrc->PixelX;
	pPointdst->PixelY = pPointsrc->PixelY;
	pPointdst->UarmX = pPointsrc->UarmX;
	pPointdst->UarmY = pPointsrc->UarmY;
	return 0;
}
/* Copy a line */
int CopyLine(t_Line* pLinedst, const t_Line* pLinesrc){
	CopyPoint(&(pLinedst->EndPoint), &(pLinesrc->EndPoint));
	CopyPoint(&(pLinedst->StartPoint), &(pLinesrc->StartPoint));
	pLinedst->PointNumber = pLinesrc->PointNumber;
	pLinedst->pPoint = pLinesrc->pPoint;
	return 0;
}
