#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>
#include <iostream>
#include "highgui.h"
#include "cv.h"
using namespace cv;
using namespace std;

#define max(x,y)  ( x>y?x:y )
#define min(x,y)  ( x<y?x:y )
#define ROIX 128
#define ROIY 96

void rgb2hsv(double R, double G, double B, double& H, double& S, double&V)
{
	double Min, Max, delta, tmp;
	R = R / 255.0;
	G = G / 255.0;
	B = B / 255.0;
	tmp = min(R, G);
	Min = min(tmp, B);
	tmp = max(R, G);
	Max = max(tmp, B);
	V = Max;

	delta = Max - Min;

	if (Max != 0)
		S = delta / Max;
	else// r = g = b = 0 // s = 0, v 灰色
	{
		S = 0;
		H = 128;
		return;
	}
	if (R == Max)
		H = (G - B) / delta;
	else if (G == Max)
		H = 2 + (B - R) / delta;
	else
		H = 4 + (R - G) / delta;

	H *= 60; // 转角度
	if (H < 0)
		H += 360;
}
void ColorSegByHSV(Mat & img, Mat & img2)
{
	int channel_num, col_num, row_num, i, j;
	double B = 0.0, G = 0.0, R = 0.0, H = 0.0, S = 0.0, V = 0.0;
	img.copyTo(img2);
	channel_num = img2.channels();
	row_num = img2.rows;
	col_num = img2.cols;

	for (i = 0; i < row_num; i++)
	{
		const double *Mi = img2.ptr <double>(i);

		for (j = 0; j < col_num; j++)
		{
			B = ((uchar*)Mi)[j*channel_num];
			G = ((uchar*)Mi)[j*channel_num + 1];
			R = ((uchar*)Mi)[j*channel_num + 2];
			//  RGB-HSV
			//rgb2hsv(R, G, B, H, S, V);
			//白色(S<0.15 && V>0.75)
			//红色相近(V >= 0.35 && S >= 0.15 && (H >= 0 && H < 15) || (H >= 340 && H < 360))//洋红：270-340
			
			rgb2hsv(R, G, B, H, S, V);
			if (V >= 0.35 && S >= 0.15 && (H >= 0 && H < 10) || (H >= 348 && H < 360))
			{
				((uchar*)Mi)[j*channel_num] = 0;  //B
				((uchar*)Mi)[j*channel_num + 1] = 0;  //G
				((uchar*)Mi)[j*channel_num + 2] = 255;  //R
			}
			else
			{
				((uchar*)Mi)[j*channel_num] = 0;  //B
				((uchar*)Mi)[j*channel_num + 1] = 0;  //G
				((uchar*)Mi)[j*channel_num + 2] = 0;  //R   

			}
		}
	}
}

void changeBW(Mat & imgo, vector <Mat> & img_sp, int num_rgb)
{
	//内核
	Mat element = getStructuringElement(0, Size(3, 3), Point(-1, -1));
	//膨胀
	dilate(img_sp[num_rgb], img_sp[num_rgb], element, cvPoint(-1, -1), 4);
	//二值化
	adaptiveThreshold(img_sp[num_rgb], img_sp[num_rgb], 255, ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 35, 30);
	//边缘
	Canny(img_sp[num_rgb], imgo, 100, 200, 3);
}
void changeMark(Mat & imgi, Mat &img, int color_num, int & dot_x, int & dot_y,int &flag)//bool能传入吗？
{
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	// 寻找轮廓
	findContours(imgi, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	// 绘出轮廓
	vector<Rect> boundRect(contours.size());
	Scalar color = Scalar(0, 255, 0);
	int rectAmax = 100;
	if (contours.size() > 1)
	{
		for (int i = 0; i < contours.size(); i++)
		{
			boundRect[i] = boundingRect(Mat(contours[i]));
			int rectW = boundRect[i].br().x - boundRect[i].tl().x;
			int rectH = boundRect[i].br().y - boundRect[i].tl().y;
			int rectA = rectH*rectW;
			if (rectA>rectAmax)
			{
				rectAmax = rectA;
				if (flag)
				{
					dot_x = boundRect[i].tl().x + (boundRect[i].br().x - boundRect[i].tl().x) / 2+(dot_x-ROIX);
					dot_y = boundRect[i].tl().y + (boundRect[i].br().y - boundRect[i].tl().y) / 2+(dot_y-ROIY);
				}
				else
				{
					dot_x = boundRect[i].tl().x + (boundRect[i].br().x - boundRect[i].tl().x) / 2 ;
					dot_y = boundRect[i].tl().y + (boundRect[i].br().y - boundRect[i].tl().y) / 2 ;
				}
			}
		}
		flag = 1;//还是标志的问题！！！！！！！！！！！！！！！！！！！
		cout << "  1   " << endl;
		CvPoint dot_center = cvPoint(dot_x, dot_y);
		if (color_num)
			circle(img, dot_center, 5, color, 2, 8, 0);
		else
			circle(img, dot_center, 5, Scalar(0, 0, 0), 2, 8, 0);
	}
	else
	{
		dot_x = 0;
		dot_y = 0;
		flag = 0;
		cout << "   0   " << endl;
		circle(img, cvPoint(480, 640), 5, color, 2, 8, 0);
	}
}
void use_roi(int dot_x,int dot_y,Mat &img,int &flag)
{
	Mat img_roi,img_red,canny_out_r;
	vector <Mat> img_sp;

}

int main()
{
	char c;
	int flag = 0;// false miss; true access
	int dot_x = 0, dot_y = 0;
	Mat img, img_red,img_red_roi, canny_out_r,canny_out_roi, result_r,img_roi,img_c,img_cc;
	vector <Mat> img_sp,img_sp_roi;
	cvNamedWindow("cam", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("result_R", CV_WINDOW_AUTOSIZE);
	//cvNamedWindow("roi", CV_WINDOW_AUTOSIZE);
	VideoCapture cap(0);
	if (!cap.isOpened())
	{
		return -1;
	}
	while (1)
	{
		cap >> img;
		const int64 startsum=getTickCount();
		//blur(img, img, Size(9, 9));
		if (flag==0)
		{
			
			ColorSegByHSV(img, img_red);//40ms
			split(img_red, img_sp);//5ms

			changeBW(canny_out_r, img_sp, 2);

			changeMark(canny_out_r, img, 1, dot_x, dot_y,flag);
		}
		
		else
		{
			
			if (dot_x>ROIX && dot_y>ROIY && dot_x<(640-ROIX) && dot_y<(480-ROIY))
			{
				cap >> img;
				Rect roi((dot_x - ROIX), (dot_y - ROIY), ROIX*2, ROIY*2);
				//Rect roi(100, 5, 200, 200);
				img_roi = img(roi);
				ColorSegByHSV(img_roi, img_red_roi);//40ms
				split(img_red_roi, img_sp_roi);//5ms

				changeBW(canny_out_roi, img_sp_roi, 2);

				changeMark(canny_out_roi, img, 1, dot_x, dot_y, flag);

			}
			else
			{
				flag = 0;
			}
			
		}

		const double secsum=(getTickCount()-startsum)/getTickFrequency();
		printf("%f\n",secsum*1000);

		cout << "mark"<<dot_x << "  " << dot_y << endl;
		imshow("result_R", canny_out_r);
		imshow("cam", img);
		//imshow("roi", canny_out_roi);
		c = cvWaitKey(10);
		if (c == 27)
			break;
	}
	cvDestroyWindow("cam");
	cvDestroyWindow("result_R");
	return 0;
}



