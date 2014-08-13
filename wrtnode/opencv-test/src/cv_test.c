#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#pragma warning(disable : 4996)     
#pragma comment(lib, "cv.lib")  
#pragma comment( lib, "cxcore.lib" )     
#pragma comment( lib, "highgui.lib" )   
int main( int argc, char** argv )
{
	char c;
	IplImage* pFrame = NULL;
	CvCapture* pCapture = NULL;
	char ImagesName[1024];
	int ImgNum=0;
	pCapture = cvCreateCameraCapture(-1);
	printf("pCapture = %d\n",pCapture);
	while(pFrame = cvQueryFrame( pCapture ))    
	{
		ImgNum=ImgNum+1;
		sprintf(ImagesName, "Image%.3d.jpg", ImgNum);
		cvSaveImage(ImagesName, pFrame,0);

		if(ImgNum > 5)
			break;
	}
	cvReleaseCapture(&pCapture);
	return 0;
}
