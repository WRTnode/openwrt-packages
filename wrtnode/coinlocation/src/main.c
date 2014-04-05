/*
	FileName	£ºmain.c
	Description	£ºmain function of coin location.
	Author		£ºSchumyHao
	Version		£ºV01
	Data		£º2013.03.30
*/

/* Uncomment next line if you want to debug the code. */
//#define DEBUG

/* Include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "coinlocation.h"

/* Define the const */
#define ARGV_FLAG_c          (0x01)
#define ARGV_FLAG_l          (0x02)
#define ARGV_FLAG_L          (0x04)
#define ARGV_FLAG_r          (0x08)
#define ARGV_FLAG_R          (0x10)
#define IS_ARGV_FLAG(FLAG)   (((FLAG) == ARGV_FLAG_c + \
										ARGV_FLAG_l + \
										ARGV_FLAG_L + \
										ARGV_FLAG_r + \
										ARGV_FLAG_R))

/* Functions Declaration*/
int ProcessArguments(int argc, char** argv, t_Plane* pWorkArea, t_Point* pCoinPoint);

#ifdef DEBUG
void *mymtom_malloc(size_t size)
{
        void *p;
        p = malloc(size);
        printf("malloc() size=%lu, p=0x%lx\n", (unsigned long)size, (unsigned long)p);
        return p;
}
#define malloc(s) mymtom_malloc(s)
#define free(p)  do {                                                   \
                printf("%s:%d:%s:free(0x%lx)\n", __FILE__, __LINE__,    \
                    __func__, (unsigned long)p);                        \
                free(p);                                                \
        } while (0)
#endif


int main(int argc, char* argv[]){
	t_Plane* pWorkArea;
	t_Point* pCoinPoint;
	/* Set coin point memory. */
	pCoinPoint = (t_Point*)malloc(sizeof(t_Point));
	/* Set working area memory. */
	pWorkArea = (t_Plane*)malloc(sizeof(t_Plane));
	if(pWorkArea == NULL){
		perror("Memory allocated wrong.\n");
		return -1;
	}
	pWorkArea->pLine = \
		(t_Line*)malloc(sizeof(t_Line)*(TOP_LEFT_UARM_Y-BOTTOM_LEFT_UARM_Y+1));
	if(pWorkArea->pLine == NULL){
		perror("Memory allocated wrong.\n");
		return -1;
	}
	pWorkArea->pLine->pPoint = \
		(t_Point*)malloc(sizeof(t_Point)*(TOP_LEFT_UARM_Y-BOTTOM_LEFT_UARM_Y+1) \
										*(BOTTOM_RIGHT_UARM_X-BOTTOM_LEFT_UARM_X+1));
	if(pWorkArea->pLine->pPoint == NULL){
		perror("Memory allocated wrong.\n");
		return -1;
	}
	/* Pass the arguments. */
	if(ProcessArguments(argc, argv, pWorkArea, pCoinPoint)<0){
		perror("Input arguments are wrong.\n");
		return -2;
	}
	/* Insert referance points's uArm location. */
	pWorkArea->BottomLeftPoint.UarmX = BOTTOM_LEFT_UARM_X;
	pWorkArea->BottomLeftPoint.UarmY = BOTTOM_LEFT_UARM_Y;
	pWorkArea->BottomRightPoint.UarmX = BOTTOM_RIGHT_UARM_X;
	pWorkArea->BottomRightPoint.UarmY = BOTTOM_RIGHT_UARM_Y;
	pWorkArea->TopLeftPoint.UarmX = TOP_LEFT_UARM_X;
	pWorkArea->TopLeftPoint.UarmY = TOP_LEFT_UARM_Y;
	pWorkArea->TopRightPoint.UarmX = TOP_RIGHT_UARM_X;
	pWorkArea->TopRightPoint.UarmY= TOP_RIGHT_UARM_Y;
	#ifdef DEBUG
	PointPrint("Bottom left point", &pWorkArea->BottomLeftPoint);
	PointPrint("Bottom right point", &pWorkArea->BottomRightPoint);
	PointPrint("Top left point", &pWorkArea->TopLeftPoint);
	PointPrint("Top right point", &pWorkArea->TopRightPoint);
	#endif
	/* Draw the working area. */
	if(RectengularAreaDraw(pWorkArea) < 0){
		perror("Area initialization wrong.\n");
		return -3;
	}
	#ifdef DEBUG
	printf("Area initialization OK.\n");
	printf("Work area's lines number is %d.\n",pWorkArea->LineNumber);
	#endif
	/* Locate the coin */
	if(CoinLocation(pCoinPoint, pWorkArea) < 0){
		perror("Coin location wrong.\n");
		return -3;
	}
	PointPrint("Coin", pCoinPoint);
	/* Wirte all point map to file:point_map. */
	if(WritePointsMap(pWorkArea) < 0){
		perror("Points map generation wrong.\n");
		return -4;
	}
	/* Free the memory. */
	free(pCoinPoint);
	free(pWorkArea->pLine->pPoint);
	free(pWorkArea->pLine);
	free(pWorkArea);
	return 0;
}
int ProcessArguments(int argc, char** argv, t_Plane* pWorkArea, t_Point* pCoinPoint){
	int i;
	int tmp;
	unsigned char Flag = 0;
	/*#ifdef DEBUG
	for(i=0; i<argc; i++){
		printf("argv[%d]:%s\n",i,argv[i]);
	}
	#endif*/
	for(i=1; i<argc; i++){
		if(*argv[i] == '-'){
			switch(*(++argv[i])){
			case 'c':
				if(Flag & ARGV_FLAG_c){
					printf("WARNING:	Got '-c' more than once in arguments\n");
				}
				else{
					tmp = atoi(argv[++i]);
					if(IS_PIXEL_X(tmp)){
						pCoinPoint->PixelX = tmp;
					}
					else{
						printf("Coin's pixelX is wrong!\n");
						return -1;
					}
					tmp = atoi(argv[++i]);
					if(IS_PIXEL_Y(tmp)){
						pCoinPoint->PixelY = tmp;
					}
					else{
						printf("Coin's pixelY is wrong!\n");
						return -1;
					}
					Flag |= ARGV_FLAG_c;
				}
				break;
			case 'l':
				if(Flag & ARGV_FLAG_l){
					printf("WARNING:	Got '-l' more than once in arguments\n");
				}
				else{
					tmp = atoi(argv[++i]);
					if(IS_BOTTOM_LEFT_PIXEL_X(tmp)){
						pWorkArea->BottomLeftPoint.PixelX = tmp;
					}
					else{
						printf("Bottom left referance point's pixelX is wrong!\n");
						return -1;
					}
					tmp = atoi(argv[++i]);
					if(IS_BOTTOM_LEFT_PIXEL_Y(tmp)){
						pWorkArea->BottomLeftPoint.PixelY = tmp;
					}
					else{
						printf("Bottom left referance point's pixelY is wrong!\n");
						return -1;
					}
					Flag |= ARGV_FLAG_l;
				}
				break;
			case 'L':
				if(Flag & ARGV_FLAG_L){
					printf("WARNING:	Got '-L' more than once in arguments\n");
				}
				else{
					tmp = atoi(argv[++i]);
					if(IS_TOP_LEFT_PIXEL_X(tmp)){
						pWorkArea->TopLeftPoint.PixelX = tmp;
					}
					else{
						printf("Top left referance point's pixelX is wrong!\n");
						return -1;
					}
					tmp = atoi(argv[++i]);
					if(IS_TOP_LEFT_PIXEL_Y(tmp)){
						pWorkArea->TopLeftPoint.PixelY = tmp;
					}
					else{
						printf("Top left referance point's pixelY is wrong!\n");
						return -1;
					}
					Flag |= ARGV_FLAG_L;
				}
				break;
			case 'r':
				if(Flag & ARGV_FLAG_r){
					printf("WARNING:	Got '-r' more than once in arguments\n");
				}
				else{
					tmp = atoi(argv[++i]);
					if(IS_BOTTOM_RIGHT_PIXEL_X(tmp)){
						pWorkArea->BottomRightPoint.PixelX = tmp;
					}
					else{
						printf("Bottom right referance point's pixelX is wrong!\n");
						return -1;
					}
					tmp = atoi(argv[++i]);
					if(IS_BOTTOM_RIGHT_PIXEL_Y(tmp)){
						pWorkArea->BottomRightPoint.PixelY = tmp;
					}
					else{
						printf("Bottom right referance point's pixelY is wrong!\n");
						return -1;
					}
					Flag |= ARGV_FLAG_r;
				}
				break;
			case 'R':
				if(Flag & ARGV_FLAG_R){
					printf("WARNING:	Got '-R' more than once in arguments\n");
				}
				else{
					tmp = atoi(argv[++i]);
					if(IS_TOP_RIGHT_PIXEL_X(tmp)){
						pWorkArea->TopRightPoint.PixelX = tmp;
					}
					else{
						printf("Top right referance point's pixelX is wrong!\n");
						return -1;
					}
					tmp = atoi(argv[++i]);
					if(IS_TOP_RIGHT_PIXEL_Y(tmp)){
						pWorkArea->TopRightPoint.PixelY = tmp;
					}
					else{
						printf("Top right referance point's pixelY is wrong!\n");
						return -1;
					}
					Flag |= ARGV_FLAG_R;
				}
				break;
			default:
				printf("WARNING:	Got redundant arguments\n");
			}
		}
	}
	if(IS_ARGV_FLAG(Flag)){
		return 0;
	}
	else{
		printf("Arguments are wrong!\n");
		return -1;
	}
}
