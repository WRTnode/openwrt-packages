/***********************************************************************
 * OpenCV 2.4.8 coinidentify TEST SAMPLE
 * By£º»ÆÓ¿£¨Daniel HWANG£©   
 * TIME:20140404
 ***********************************************************************/
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  

#include <opencv2/calib3d/calib3d.hpp>   //Camera calibration lib

#include <cctype>
#include <stdio.h>
#include <string.h>

using namespace std;
using namespace cv;

void Image_FindCycle(Mat& inputImage);  //Gray image matching for circle

void SwitchPattern1();     //IDENTIFY COIN WITH CAMERA
void SwitchPattern2();    //BACK UP FUNCTION
void SwitchPattern3();    //IDENTIFY COIN WITH PICTURE
enum { DETECTION = 0, CAPTURING = 1, CALIBRATED = 2 };
enum SwitchMode { SWITCHMODE1, SWITCHMODE2, SWITCHMODE3 };  //Mode switch

double TIMETEST=0;   //TIMETEST

int main( int argc, char** argv )  
{
	SwitchMode switchmode = SWITCHMODE3;   //Default start first boot mode
	switch( switchmode )     //In what form OF the identification mode, the switch COULD BE very USEFUL!
    {
        case SWITCHMODE1:         //IDENTIFY COIN WITH CAMERA
            SwitchPattern1();
            break;
        case SWITCHMODE2:      //BACK UP FUNCTION
			SwitchPattern2();
            break;
        case SWITCHMODE3:      //IDENTIFY COIN WITH PICTURE
			SwitchPattern3();
            break;
        default:
            return fprintf( stderr, "Unknown pattern type\n" ), -1;   
    }	
    return 0;  
} 

int circles_size_old=0;   //Global variables, the last number OF searching cycle.
int circles_found[20][3]={0};  //Storage or container of found round
int cycle_time=0;
void SwitchPattern1()   //IDENTIFY COIN WITH CAMERA
{
	int i;   

	VideoCapture capture(0);
	for(i = 0;;i++)   //Forever in this infinite loop.
    {
		Mat view;
		bool blink = false;
		if( capture.isOpened() )  
		{
			Mat view0;
			capture >> view0;
			view0.copyTo(view);    //Do a cache
		}
		cvtColor(view, view, CV_BGR2GRAY);    //GRAY IMAGE.
		if(cycle_time==10)
		{
			i=0;
		}
		Image_FindCycle(view); 
		cycle_time=i;

		//if(waitKey(30) >=0)     //for window
			//break;              //for window
	}
}

void SwitchPattern2()   // BACK UP FUNCTION
{

}


void SwitchPattern3()    //IDENTIFY COIN PICTURE
{
	const char* imagename = "/tmp/v.jpg";//Read A image from a file	
	Mat XV2 = imread(imagename);   //read A color image
	//imshow("COLOR IMAGE",PictureDiff1);  
	cv::cvtColor(XV2, XV2, CV_BGR2GRAY);
	//imshow("GRAY IMAGE",PictureDiff1); 
  	Image_FindCycle(XV2); 
    //waitKey(0);     //for window
	//while(1);      //for window
    //return 0;     //for window
}


void Image_FindCycle(Mat& inputImage)  //Gray image matching for circle
{
	//Circle detection     //Attention! detection is gray
	//GaussianBlur(view,view,Size(5,5),1.5);  //Smoothing image   //eliminate interference
	std::vector<Vec3f> circles;   //Storage or container of cycle
	//Hough transform circle detection 
	//Parameters: detecting image, detecting results, detection method£¨not only parameter£©
	             //Accumulator resolution, the distance between the two round£¬
				 //The canny threshold limit (lower bound is automatically set to the upper half)£¬
				 //The minimum required number of voting center, maximum and minimum radius  
	HoughCircles(inputImage,circles,CV_HOUGH_GRADIENT,2,50,200,100,10,100); //2,50); 
	//std::cout<<"circles number:"<<circles.size()<<"¸ö"<<std::endl;     //for window
	int j=0,rsum=0,rmean=0; 

	//This is a not adaptive algorithm
	if(circles_size_old<int(circles.size()))   //The goal: as far as possible the recognition algorithm of coins
	{
		circles_size_old=int(circles.size());
		for(j=0;j<circles_size_old;j++)
		{
			circles_found[j][0]=int(circles[j][0]);
			circles_found[j][1]=int(circles[j][1]);
			circles_found[j][2]=int(circles[j][2]);
		}
		j=0;
	}
	rmean=44;   //the contant figure is for defying one yuan or fifty cents directly.

	if(cycle_time==0)
	{
		//std::cout<<"circles number:"<<circles_size_old<<"¸ö"<<std::endl;   //for window
		for(j=0;j<circles_size_old;j++)  		//Draw a circle round and output the position  
		{  
			//circle(inputImage,Point((*itc)[0],(*itc)[1]),(*itc)[2],Scalar(255),2);
			if(circles_found[j][2]>=rmean)
			{
				std::cout<<"One Yuan : "<< circles_found[j][0]<<","<<circles_found[j][1]<<std::endl;
			}
			else{
				std::cout<<"Fifty Cents : "<< circles_found[j][0]<<","<<circles_found[j][1]<<std::endl;
			}
		}  
		j=0;
		cycle_time=0;
	}


	/*	
	for(j=0;j<circles.size();j++)  		//Draw a circle round and output the position 
	{  
		//circle(inputImage,Point((*itc)[0],(*itc)[1]),(*itc)[2],Scalar(255),2);
		if(circles[j][2]>=rmean)
		{
			std::cout<<"One Yuan : "<< circles[j][0]<<","<<circles[j][1]<<std::endl;
		}
		else{
			std::cout<<"Fifty Cents : "<< circles[j][0]<<","<<circles[j][1]<<std::endl;
		}
	}  
	j=0;*/
	//imshow("COIN detection on gray image",inputImage);    //for window
	

	/*    //This is a adaptive algorithm
	if(circles.size()>1)   //Adaptive algorithm on one yuan or fifty cents
	{
		std::vector<Vec3f>::const_iterator itc = circles.begin(); 
		while(itc != circles.end())  		//adaptive algorithm 
		{  
			circle(inputImage,Point((*itc)[0],(*itc)[1]),(*itc)[2],Scalar(255),2);
			//std::cout<<"the centre and radius of coin"<< circles[j]<<std::endl;
			rsum+=(int)circles[j][2];
			++itc;j++;  
		}  
		j=0;
		rmean =(int)(rsum/circles.size());
	}
	else{rmean=0;}
	*/	
}