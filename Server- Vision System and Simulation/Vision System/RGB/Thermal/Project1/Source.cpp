#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib") //Required for WinSock
#include <WinSock2.h> //For win sockets
#include <string> //For std::string
#include <iostream> //For std::cout, std::endl, std::cin.getline

#include "C:\\Users\Midtnight\source\repos\Project1\stdafx.h"
#include <stdlib.h>
#include <cstdio>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <PvApi.h>

using namespace cv;
using namespace std;

typedef struct
{
	unsigned long   UID;
	tPvHandle       Handle;
	tPvFrame        Frame;

} tCamera;

SOCKET Connection;//This client's connection to the server
std::string rmsg;

Mat src = Mat(src.rows, src.cols, CV_8UC3);
Mat src_gray = Mat(src.rows, src.cols, CV_8UC3);
Mat canny_output = Mat(src.rows, src.cols, CV_8UC3);
Mat RGB = Mat(src.rows, src.cols, CV_8UC3);

int thresh = 255, max_thresh = 255; //treshold
int k = 0, j = 0, j2 = 0, x = 0, x2 = 0, y = 0, y2 = 0, z = 1; //For counting
int Buffer = 0, Colm = 0; //Buffer for the eggs
int test1 = 50, test2 = 100, test3 = 175, test4 = 858, //tresholds for the catagorise
test5 = 50, test6 = 0, test7 = 175, z2 = 1, test8 = 100; //tresholds for the catagorise
int plusMinus = 25, StartData = 0, Mean = 0; //Control
int testmode = 0; //For testing

double gridwidth = 62, gridhight = 69, gridwidth1 = 62, gridhight1 = 69; //For the grids

char c;
RNG rng(12345);

int goodEggs[2], deadEggs[2], clearEggs[2], noEggs[2]; //Counting eggs
int PointsX[2][100], PointsY[2][100]; //Cordinate
int noEggs1[2][10], goodEggs1[2][10], deadEggs1[2][10], clearEggs1[2][10]; //catagorizing the eggs.
int PickUP[2][1][5][7]; //For pick the eggs.
int Mean1[4] = { 0,0,0 }, Mean2[4] = { 0,0,0 }; //Mean

char buffer[256]; //Create buffer to hold messages up to 256 characters
char eggchar[256] = ""; //Create array for the eggs
						//int integg[10]; // Optional

//Setup of RGB 
void initialize_camera(tCamera* current_cam, int max_capture_width, int max_capture_height, int desired_width, int desired_height) {
	// Initialize the PvAPI interface so that we can look for cameras
	if (!PvInitialize()) {
		// Wait for the response from a camera after the initialization of the driver
		// This is done by checking if camera's are found yet
		while (PvCameraCount() == 0)
		{
			waitKey(15);
		}

		// If there is a camera connecte to the camera 1 interface, grab it!
		tPvCameraInfo cameraInfo;
		if (PvCameraList(&cameraInfo, 1, NULL) == 1)
		{
			unsigned long frameSize;

			// Get the camera ID
			current_cam->UID = cameraInfo.UniqueId;
			// Open the camera
			if (!PvCameraOpen(current_cam->UID, ePvAccessMaster, &(current_cam->Handle)))
			{
				// Debug
				cout << "Camera opened succesfully" << endl;

				// Get the image size of every capture
				PvAttrUint32Get(current_cam->Handle, "TotalBytesPerFrame", &frameSize);
				cout << "Framesize: " << frameSize << endl;

				// Allocate a buffer to store the image
				memset(&current_cam->Frame, 0, sizeof(tPvFrame));
				current_cam->Frame.ImageBufferSize = frameSize;
				current_cam->Frame.ImageBuffer = new char[frameSize];

				// Set maximum camera parameters - camera specific
				// Code will generate an input window from the center with the size you want
				int center_x = max_capture_width / 2;
				int center_y = max_capture_height / 2;

				// Set the manta camera parameters to get wanted frame size retrieved
				PvAttrUint32Set(current_cam->Handle, "RegionX", center_x - (desired_width / 2));
				PvAttrUint32Set(current_cam->Handle, "RegionY", center_y - (desired_height / 2));
				PvAttrUint32Set(current_cam->Handle, "Width", desired_width);
				PvAttrUint32Set(current_cam->Handle, "Height", desired_height);

				// Start the camera
				PvCaptureStart(current_cam->Handle);

				// Set the camera to capture continuously
				PvAttrEnumSet(current_cam->Handle, "AcquisitionMode", "Continuous");
				PvCommandRun(current_cam->Handle, "AcquisitionStart");
				PvAttrEnumSet(current_cam->Handle, "FrameStartTriggerMode", "Freerun");
			}
			else {
				cout << "Opening camera error" << endl;
			}
		}
		else {
			cout << "Camera not found or opened unsuccesfully" << endl;
		}
	}
	else {
		// State that we did not succeed in initializing the API
		cout << "Failed to initialise the camera API" << endl;
	}
}
//Take thermal picture
void thermal() {
	VideoCapture cap("http://192.168.1.70/snapshot.jpg");
	if (!cap.isOpened())
	{
		cout << "Camera not found" << endl;
		getchar();
		return;
	}

	while (cap.isOpened())
	{
		cap >> src;
		if (src.empty()) break;

		//imshow("video", src);
		//waitKey(20);
		if (waitKey(30) >= 0) break;
		imwrite("C:\\Users\\Midtnight\\Desktop\\Thermal.jpg", src);
		cvWaitKey(0);
	}
}
//Take RGB picture
void rgb() {
	tCamera		myCamera;
	tPvErr		Errcode;

	// Be sure to move the windows to correct location
	// This is done to ensure that the window takes as much of the screen as possible, but can be commented out
	//namedWindow("View window", 1);
	//moveWindow("View window", 50, 50);

	// Initialize the camera API and perform some checks
	int max_capture_width = 1936;
	int max_capture_height = 1216;
	int desired_width = 640;
	int desired_height = 480;

	if (k == 1) { goto LOOP; }
	initialize_camera(&myCamera, max_capture_width, max_capture_height, desired_width, desired_height);
	k = 1;

	// Create infinite loop - break out when condition is met
	// This is done for trigering the camera capture
LOOP:
	if (!PvCaptureQueueFrame(myCamera.Handle, &(myCamera.Frame), NULL))
	{
		while (PvCaptureWaitForFrameDone(myCamera.Handle, &(myCamera.Frame), 100) == ePvErrTimeout)
		{
		}

		////////////////////////////////////////////////////////
		// Here comes the OpenCV functionality for each frame //
		////////////////////////////////////////////////////////

		// Create an image header (mono image)
		// Push ImageBuffer data into the image matrix

		Mat image = Mat(myCamera.Frame.Height, myCamera.Frame.Width, CV_8U);
		image.data = (uchar *)myCamera.Frame.ImageBuffer;

		// Convert from Bayer Pattern (single channel) to BGR colour image

		Mat color = Mat(myCamera.Frame.Height, myCamera.Frame.Width, CV_8UC3);
		cvtColor(image, color, CV_BayerBG2BGR);

		// Show the actual frame
		// Wait 10 ms to have an actual window visualisation
		//imshow("View window", color);
		//waitKey(20);
		imwrite("C:\\Users\\Midtnight\\Desktop\\RGB.jpg", color);
		// Release the image data
		//image.release();
	}
}
//Make grid on thermal picture
void gridThermal() {
	//----------------Make Grid------------------
	int width = src.size().width;
	int height = src.size().height;

	for (int i = 0; i < height; i += gridhight) {
		if (y == 1) { i = i - 31; }
		if (i == 3
			|| y == 3
			|| y == 4) {
			cv::line(src, Point(0, i), Point(width, i), cv::Scalar(0, 0, 255), 9);
		}
		y++;
	}
	for (int i = 0; i < width; i += gridwidth) {
		if (x == 1) { i = i + 115; }
		if (x == 2) { i = i + 5; }
		if (x == 3) { i = i + 10; }
		if (x == 4) { i = i + 5; }
		if (x == 5) { i = i + 5; }
		if (x == 6) { i = i + 5; }
		if (x == 7) { i = i; }
		if (x == 8) { i = i + 50; }
		if (i == 3
			|| x == 1
			|| x == 2
			|| x == 3
			|| x == 4
			|| x == 5
			|| x == 6
			|| x == 7) {
			cv::line(src, Point(i, 0), Point(i, height), cv::Scalar(0, 0, 255), 9);
		}

		x++;
	}
	imwrite("C:\\Users\\Midtnight\\Desktop\\test.jpg", src);
}
//Make grid on RGB picture
void gridRGB() {
	//----------------Make Grid------------------
	int width = RGB.size().width;
	int height = RGB.size().height;

	for (int i = 0; i < height; i += gridhight1) {
		if (y2 == 2) { i = i + 30; }
		if (y2 == 4) { i = i - 40; }
		if (i == 3
			|| y2 == 2
			|| y2 == 4) {
			cv::line(RGB, Point(0, i), Point(width, i), cv::Scalar(0, 0, 255), 9);
		}
		y2++;
	}
	for (int i = 0; i < width; i += gridwidth1) {
		if (x2 == 1) { i = i; }
		if (x2 == 2) { i = i + 20; }
		if (x2 == 3) { i = i + 25; }
		if (x2 == 4) { i = i + 20; }
		if (x2 == 5) { i = i + 20; }
		if (x2 == 6) { i = i + 15; }
		if (x2 == 7) { i = i + 25; }
		if (x2 == 8) { i = i; }
		if (i == 3
			|| x2 == 1
			|| x2 == 2
			|| x2 == 3
			|| x2 == 4
			|| x2 == 5
			|| x2 == 6
			|| x2 == 7) {
			cv::line(RGB, Point(i, 0), Point(i, height), cv::Scalar(0, 0, 255), 9);
		}
		x2++;
	}
	imwrite("C:\\Users\\Midtnight\\Desktop\\test3.jpg", RGB);
}
//Find the midpoint in the grid on the thermal picture
void midPointThermal() {
	/// Create Window
	char* source_window = "Source";
	//namedWindow(source_window, WINDOW_AUTOSIZE);
	//imshow(source_window, src);
	//waitKey(20);

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	//cvtColor(src, src_gray, COLOR_BGR2GRAY);
	//imwrite("C:\\Users\\Midtnight\\Desktop\\src.jpg", src);
	blur(src, src_gray, Size(3, 3));
	//imwrite("C:\\Users\\Midtnight\\Desktop\\blur.jpg", src_gray);
	/// Detect edges using canny
	Canny(src_gray, canny_output, thresh, thresh * 2, 3);
	//imwrite("C:\\Users\\Midtnight\\Desktop\\canny.jpg", canny_output);
	/// Find contours
	findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	//imwrite("C:\\Users\\Midtnight\\Desktop\\contours.jpg", contours);

	/// Get the moments
	vector<Moments> mu(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		mu[i] = moments(contours[i], false);
	}

	///  Get the mass centers:
	vector<Point2f> mc(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		if (i == 13 || i == 15 || i == 18 || i == 20 || i == 10 || i == 12) {
			mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
			PointsX[0][z] = mu[i].m10 / mu[i].m00;
			PointsY[0][z] = mu[i].m01 / mu[i].m00;
			j++, z++;
		}
	}

	/// Draw contours
	Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
	for (int i = 0; i < contours.size(); i++)
	{
		Scalar color = Scalar(0, 0, 255);
		drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
		if (i == 13 || i == 15 || i == 18 || i == 20 || i == 10 || i == 12) {
			circle(drawing, mc[i], 0, color, 4, 0, 0);
		}
	}

	/// Show in a window
	//namedWindow("Contours", WINDOW_AUTOSIZE);
	imwrite("C:\\Users\\Midtnight\\Desktop\\test2.jpg", drawing);
	//imshow("Contours", drawing);
	//waitKey(20);

	/*/// Calculate the area with the moments 00 and compare with the result of the OpenCV function
	printf("\t Info: Area and Contour Length \n");
	for (int i = 0; i < contours.size(); i++)
	{
		printf(" * Contour[%d] - Area (M_00) = %.2f - Area OpenCV: %.2f - Length: %.2f \n",
		i, mu[i].m00, contourArea(contours[i]), arcLength(contours[i], true));
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		//drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
		//circle(drawing, mc[i], 4, color, -1, 8, 0);
	}*/
}
//Find the midpoint in the grid on the RGB picture
void midPointRGB() {
	/// Create Window
	char* source_window = "Source";
	//namedWindow(source_window, WINDOW_AUTOSIZE);
	//imshow(source_window, src);
	//waitKey(20);

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	//cvtColor(src, src_gray, COLOR_BGR2GRAY);
	//imwrite("C:\\Users\\Midtnight\\Desktop\\src.jpg", src);
	blur(src, src_gray, Size(3, 3));
	//imwrite("C:\\Users\\Midtnight\\Desktop\\blur.jpg", src_gray);
	/// Detect edges using canny
	Canny(src_gray, canny_output, thresh, thresh * 2, 3);
	blur(canny_output, canny_output, Size(3, 3));
	//imwrite("C:\\Users\\Midtnight\\Desktop\\canny.jpg", canny_output);
	/// Find contours
	findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	//imwrite("C:\\Users\\Midtnight\\Desktop\\contours.jpg", contours);

	/// Get the moments
	vector<Moments> mu(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		mu[i] = moments(contours[i], false);
	}

	///  Get the mass centers:
	vector<Point2f> mc(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		if (i == 15 || i == 18 || i == 19 || i == 9 || i == 11 || i == 23) {
			mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
			PointsX[1][z2] = mu[i].m10 / mu[i].m00;
			PointsY[1][z2] = mu[i].m01 / mu[i].m00;
			j2++, z2++;
		}
	}

	/// Draw contours
	Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
	for (int i = 0; i < contours.size(); i++)
	{
		Scalar color = Scalar(0, 0, 255);
		drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
		if (i == 15 || i == 18 || i == 19 || i == 9 || i == 11 || i == 23) {
			circle(drawing, mc[i], 0, color, 4, 0, 0);
		}
	}

	/// Show in a window
	//namedWindow("Contours", WINDOW_AUTOSIZE);
	imwrite("C:\\Users\\Midtnight\\Desktop\\test4.jpg", drawing);
	//imshow("Contours", drawing);
	//waitKey(20);

	/*/// Calculate the area with the moments 00 and compare with the result of the OpenCV function
	printf("\t Info: Area and Contour Length \n");
	for (int i = 0; i < contours.size(); i++)
	{
	printf(" * Contour[%d] - Area (M_00) = %.2f - Area OpenCV: %.2f - Length: %.2f \n",
	i, mu[i].m00, contourArea(contours[i]), arcLength(contours[i], true));
	Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	//drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
	//circle(drawing, mc[i], 4, color, -1, 8, 0);
	}*/
}
//Merge taken picture with the grid, for test.
void Merge() {
	//--------------------Merge-------------------
	double alpha = 0.5; double beta; double input;
	Mat src1, src2, dst;
	/// Read image ( same size, same type )
	src1 = src;
	src2 = imread("C:\\Users\\Midtnight\\Desktop\\test2.jpg");

	/// Create Windows
	beta = (1.0 - alpha);
	addWeighted(src1, alpha, src2, beta, 0.0, dst);
	imwrite("C:\\Users\\Midtnight\\Desktop\\test3.jpg", dst);
	//--------------------------------------------------*/
}
//Find the pixel mean in thermal
void meanValThermal(Mat src, int i) {
	for (int k = 0; k <= 0; k++)
	{
		for (int j = 0; j <= 20; j++)
		{
			Mean1[k] = Mean1[k] + src.at<Vec3b>(Point(PointsX[0][i] - j, PointsY[0][i] - j))[k];
			//src.at<Vec3b>(Point(PointsX[0][i] - j, PointsY[0][i] - j))[k] = 255;
			Mean1[k] = Mean1[k] + src.at<Vec3b>(Point(PointsX[0][i], PointsY[0][i] - j))[k];
			//src.at<Vec3b>(Point(PointsX[0][i], PointsY[0][i] - j))[k] = 255;
			Mean1[k] = Mean1[k] + src.at<Vec3b>(Point(PointsX[0][i] + j, PointsY[0][i] - j))[k];
			//src.at<Vec3b>(Point(PointsX[0][i] + j, PointsY[0][i] - j))[k] = 255;

			Mean1[k] = Mean1[k] + src.at<Vec3b>(Point(PointsX[0][i] - j, PointsY[0][i]))[k];
			//src.at<Vec3b>(Point(PointsX[0][i] - j, PointsY[0][i]))[k] = 255;
			Mean1[k] = Mean1[k] + src.at<Vec3b>(Point(PointsX[0][i], PointsY[0][i]))[k];
			//src.at<Vec3b>(Point(PointsX[0][i], PointsY[0][i]))[k] = 255;
			Mean1[k] = Mean1[k] + src.at<Vec3b>(Point(PointsX[0][i] + j, PointsY[0][i]))[k];
			//src.at<Vec3b>(Point(PointsX[0][i] + j, PointsY[0][i]))[k] = 255;

			Mean1[k] = Mean1[k] + src.at<Vec3b>(Point(PointsX[0][i] - j, PointsY[0][i] + j))[k];
			//src.at<Vec3b>(Point(PointsX[0][i] - j, PointsY[0][i] + j))[k] = 255;
			Mean1[k] = Mean1[k] + src.at<Vec3b>(Point(PointsX[0][i], PointsY[0][i] + j))[k];
			//src.at<Vec3b>(Point(PointsX[0][i], PointsY[0][i] + j))[k] = 255;
			Mean1[k] = Mean1[k] + src.at<Vec3b>(Point(PointsX[0][i] + j, PointsY[0][i] + j))[k];
			//src.at<Vec3b>(Point(PointsX[0][i] + j, PointsY[0][i] + j))[k] = 255;
			Mean1[k] = (Mean1[k] / (9));
		}
	}
}
//Find the pixel mean in RGB
void meanValRGB(Mat RGB, int i) {
	for (int k = 0; k <= 2; k++)
	{
		for (int j = 0; j <= 20; j++)
		{
			Mean2[k] = Mean2[k] + RGB.at<Vec3b>(Point(PointsX[1][i] - j, PointsY[1][i] - j))[k];
			//RGB.at<Vec3b>(Point(PointsX[1][i] - j, PointsY[1][i] - j))[k] = 0;
			Mean2[k] = Mean2[k] + RGB.at<Vec3b>(Point(PointsX[1][i], PointsY[1][i] - j))[k];
			//RGB.at<Vec3b>(Point(PointsX[1][i], PointsY[1][i] - j))[k] = 0;
			Mean2[k] = Mean2[k] + RGB.at<Vec3b>(Point(PointsX[1][i] + j, PointsY[1][i] - j))[k];
			//RGB.at<Vec3b>(Point(PointsX[1][i] + j, PointsY[1][i] - j))[k] = 0;

			Mean2[k] = Mean2[k] + RGB.at<Vec3b>(Point(PointsX[1][i] - j, PointsY[1][i]))[k];
			//RGB.at<Vec3b>(Point(PointsX[1][i] - j, PointsY[1][i]))[k] = 0;
			Mean2[k] = Mean2[k] + RGB.at<Vec3b>(Point(PointsX[1][i], PointsY[1][i]))[k];
			//RGB.at<Vec3b>(Point(PointsX[1][i], PointsY[1][i]))[k] = 0;
			Mean2[k] = Mean2[k] + RGB.at<Vec3b>(Point(PointsX[1][i] + j, PointsY[1][i]))[k];
			//RGB.at<Vec3b>(Point(PointsX[1][i] + j, PointsY[1][i]))[k] = 0;

			Mean2[k] = Mean2[k] + RGB.at<Vec3b>(Point(PointsX[1][i] - j, PointsY[1][i] + j))[k];
			//RGB.at<Vec3b>(Point(PointsX[1][i] - j, PointsY[1][i] + j))[k] = 0;
			Mean2[k] = Mean2[k] + RGB.at<Vec3b>(Point(PointsX[1][i], PointsY[1][i] + j))[k];
			//RGB.at<Vec3b>(Point(PointsX[1][i], PointsY[1][i] + j))[k] = 0;
			Mean2[k] = Mean2[k] + RGB.at<Vec3b>(Point(PointsX[1][i] + j, PointsY[1][i] + j))[k];
			//RGB.at<Vec3b>(Point(PointsX[1][i] + j, PointsY[1][i] + j))[k] = 0;
			Mean2[k] = (Mean2[k] / (9));
		}
	}
}
//Compare mean and thresholds in from thermal
void CheckThermal(int i, int Mean) {
	if (PointsX[i] != 0 && PointsY[i] != 0)
	{
		//---------------------------------------------------------
		if (171 <= Mean && Mean <= 174){
			PickUP[0][Buffer][Colm][i] = 49; //1
			deadEggs[0]++; //cout << "Dead";
			deadEggs1[0][deadEggs[0]] = Mean;

			if (deadEggs[0] % 10 == 0) {
				test3 = 0;
				for (int i = 0; i <= 10; i++)
				{
					test3 = (test3 + deadEggs1[0][i]);
				}
				test3 = test3 / 10;
				deadEggs[0] = 0;
			}
		} //thresh hold for dead in thermal pic.
		  //---------------------------------------------------------
		  //--------------------------------------------------------
		else {
			PickUP[0][Buffer][Colm][i] = 48; //0
			goodEggs[0]++; //cout << "Alive";
			goodEggs1[0][goodEggs[0]] = Mean;

			if (goodEggs[0] % 10 == 0) {
				test2 = 0;
				for (int i = 0; i <= 10; i++)
				{
					test2 = (test2 + goodEggs1[0][i]);
				}
				test2 = test2 / 10;
				goodEggs[0] = 0;
			}
		} //thresh hold for Good in thermal pic.
	}
}
//Compare mean and thresholds in from RGB
void CheckRGB(int i, int Mean) {
	if (PointsX[i] != 0 && PointsY[i] != 0)
	{
		if (Mean == test4) {
			for (int o = i; o <= 6; o++)
			{
				PickUP[1][Buffer][Colm][o] = 51; //3
			}
			noEggs[1]++; //cout << "No";
			noEggs1[1][noEggs[1]] = Mean;

			if (noEggs[1] % 10 == 0) {
				test4 = 0;
				for (int i = 0; i <= 10; i++)
				{
					test4 = (test4 + noEggs1[1][i]);
				}
				test4 = test4 / 10;
				noEggs[1] = 0;
			}
		}//thresh hold for no egg in RBG pic.
		//---------------------------------------------------------
		else if (92 <= Mean && Mean <= 112) {
			PickUP[1][Buffer][Colm][i] = 49; //1
			deadEggs[1]++; //cout << "Dead";
			deadEggs1[1][deadEggs[1]] = Mean;

			if (deadEggs[1] % 10 == 0) {
				test5 = 0;
				for (int i = 0; i <= 10; i++)
				{
					test5 = (test5 + deadEggs1[1][i]);
				}
				test5 = test5 / 10;
				deadEggs[1] = 0;
			}
		}//thresh hold for dead egg in RBG pic.
		 //---------------------------------------------------------
		else if (830 < Mean && Mean < 858 || 390 < Mean && Mean <= 413 ||
			714 < Mean && Mean < 725 || 725 < Mean && Mean < 734) {
			PickUP[1][Buffer][Colm][i] = 50; //2
			clearEggs[1]++; //cout << "Clear";
			clearEggs1[1][clearEggs[1]] = Mean;

			if (clearEggs[1] % 10 == 0) {
				test7 = 0;
				for (int i = 0; i <= 10; i++)
				{
					test7 = (test7 + goodEggs1[1][i]);
				}
				test7 = test7 / 10;
				clearEggs[1] = 0;
			}
		} //thresh hold for clear in RBG pic.*/
		  //--------------------------------------------------------
		else {
			PickUP[1][Buffer][Colm][i] = 48; //0
			goodEggs[1]++; //cout << "Fertilized";
			goodEggs1[1][goodEggs[1]] = Mean;

			if (goodEggs[1] % 10 == 0) {
				test6 = 0;
				for (int i = 0; i <= 10; i++)
				{
					test6 = (test6 + goodEggs1[1][i]);
				}
				test6 = test6 / 10;
				goodEggs[1] = 0;
			}
		} //thresh hold for Good in thermal pic.
	}
}
//Catagorize the eggs and send the data to the server.
void ClientThread()
{
	while (true)
	{
		recv(Connection, buffer, sizeof(buffer), NULL); //receive buffer
														//std::cout << buffer << std::endl; //print out buffer
		rmsg = buffer;

		if (rmsg == "RBGStart") {
			for (int i = 1; i <= 5; i++)
			{
				//-------------Thermal Cam---------------------
				//thermal();
				//src = imread("C:\\Users\\Midtnight\\Desktop\\Thermal.jpg");
				if (testmode == 0) {
					if (i == 1) { src = imread("C:\\Users\\Midtnight\\Desktop\\Thermal\\test3-45min\\Thermal1.jpg"); }
					if (i == 2) { src = imread("C:\\Users\\Midtnight\\Desktop\\Thermal\\test3-45min\\Thermal2.jpg"); }
					if (i == 3) { src = imread("C:\\Users\\Midtnight\\Desktop\\Thermal\\test3-45min\\Thermal3.jpg"); }
					if (i == 4) { src = imread("C:\\Users\\Midtnight\\Desktop\\Thermal\\test3-45min\\Thermal4.jpg"); }
					if (i == 5) { src = imread("C:\\Users\\Midtnight\\Desktop\\Thermal\\test3-45min\\Thermal5.jpg"); }
				}
				if (testmode == 1) {
					if (i == 1) { src = imread("C:\\Users\\Midtnight\\Desktop\\Flipped-pictures-test3\\Thermal\\Flip1\\Thermal1F1.jpg"); }
					if (i == 2) { src = imread("C:\\Users\\Midtnight\\Desktop\\Flipped-pictures-test3\\Thermal\\Flip1\\Thermal2F1.jpg"); }
					if (i == 3) { src = imread("C:\\Users\\Midtnight\\Desktop\\Flipped-pictures-test3\\Thermal\\Flip1\\Thermal3F1.jpg"); }
					if (i == 4) { src = imread("C:\\Users\\Midtnight\\Desktop\\Flipped-pictures-test3\\Thermal\\Flip1\\Thermal4F1.jpg"); }
					if (i == 5) { src = imread("C:\\Users\\Midtnight\\Desktop\\Flipped-pictures-test3\\Thermal\\Flip1\\Thermal5F1.jpg"); }
				}
				//---------------------RGB---------------------
				//rgb();
				//Mat color = imread("C:\\Users\\Midtnight\\Desktop\\RGB.jpg");
				if (testmode == 0) {
					if (i == 1) { RGB = imread("C:\\Users\\Midtnight\\Desktop\\RGB\\test3-45min\\RGB1.jpg"); }
					if (i == 2) { RGB = imread("C:\\Users\\Midtnight\\Desktop\\RGB\\test3-45min\\RGB2.jpg"); }
					if (i == 3) { RGB = imread("C:\\Users\\Midtnight\\Desktop\\RGB\\test3-45min\\RGB3.jpg"); }
					if (i == 4) { RGB = imread("C:\\Users\\Midtnight\\Desktop\\RGB\\test3-45min\\RGB4.jpg"); }
					if (i == 5) { RGB = imread("C:\\Users\\Midtnight\\Desktop\\RGB\\test3-45min\\RGB5.jpg"); }
				}
				if (testmode == 1) {
					if (i == 1) { RGB = imread("C:\\Users\\Midtnight\\Desktop\\Flipped-pictures-test3\\RGB\\Flip1\\RGB1F1.jpg"); }
					if (i == 2) { RGB = imread("C:\\Users\\Midtnight\\Desktop\\Flipped-pictures-test3\\RGB\\Flip1\\RGB2F1.jpg"); }
					if (i == 3) { RGB = imread("C:\\Users\\Midtnight\\Desktop\\Flipped-pictures-test3\\RGB\\Flip1\\RGB3F1.jpg"); }
					if (i == 4) { RGB = imread("C:\\Users\\Midtnight\\Desktop\\Flipped-pictures-test3\\RGB\\Flip1\\RGB4F1.jpg"); }
					if (i == 5) { RGB = imread("C:\\Users\\Midtnight\\Desktop\\Flipped-pictures-test3\\RGB\\Flip1\\RGB5F1.jpg"); }
				}
				//------------------For pick eggs-----------------------
				//Merge();
				for (int i = 1; i <= 6; i++)
				{
					//----------------------Thermal Check--------------------------
					Mean = 0; meanValThermal(src, i); /*Thermal*/
					//imwrite("C:\\Users\\Midtnight\\Desktop\\test10.jpg", src);
					Mean = Mean1[0];
					//if (testmode != 0) cout << Mean; cout << " ";
					//if (testmode != 0) { cout << "\n"; cout << "-----------------------"; cout << "\n"; }
					CheckThermal(i, Mean);

					//------------------------RGB Check------------------------------
					Mean = 0; meanValRGB(RGB, i); /*RGB*/
					//imwrite("C:\\Users\\Midtnight\\Desktop\\test11.jpg", RGB);
					Mean = Mean2[0] + Mean2[1] + Mean2[2];
					if (testmode != 0) cout << Mean; cout << " ";
					if (testmode != 0) if (i % 6 == 0) { cout << "\n"; cout << "-----------------------"; cout << "\n"; }
					CheckRGB(i, Mean);

					//----------------------------------------------------------------
					eggchar[0] = 83;
					//----------------------------------------------------------------
					//If two good eggs = good egg.
					if (PickUP[0][Buffer][Colm][i] == 48 && PickUP[1][Buffer][Colm][i] == 48) {
						eggchar[i] = 48; if (testmode == 0) cout << eggchar[i]; cout << " ";
					}
					//if good and if not good egg = good egg. //new
					else if (PickUP[0][Buffer][Colm][i] == 49 && PickUP[1][Buffer][Colm][i] == 48 ) {
						eggchar[i] = 49; if (testmode == 0) cout << eggchar[i]; cout << " ";
					}
					//if not good and if good egg = good egg.
					else if (PickUP[0][Buffer][Colm][i] != 51 && PickUP[1][Buffer][Colm][i] == 50) {
						eggchar[i] = 50; if (testmode == 0) cout << eggchar[i]; cout << " ";
					}
					//if no egg and if not no egg = no egg
					else if (PickUP[0][Buffer][Colm][i] != 51 && PickUP[1][Buffer][Colm][i] == 51 
						&& PickUP[1][Buffer][Colm][i] != 50) {
						eggchar[i] = 51; if (testmode == 0) cout << eggchar[i]; cout << " ";
					}
					//if not no egg and if no egg = no egg.
					else if (PickUP[0][Buffer][Colm][i] == 51 && PickUP[1][Buffer][Colm][i] == 51) {
						eggchar[i] = 51; if (testmode == 0) cout << eggchar[i]; cout << " ";
					}
					//if dead egg and good egg = dead egg.
					else if (PickUP[0][Buffer][Colm][i] == 49 && PickUP[1][Buffer][Colm][i] == 49) {
						eggchar[i] = 49; if (testmode == 0) cout << eggchar[i]; cout << " ";
					}
					//if not dead egg and dead egg = dead egg.
					else if (PickUP[0][Buffer][Colm][i] != 51 && PickUP[1][Buffer][Colm][i] == 49) {
						eggchar[i] = 49; if (testmode == 0) cout << eggchar[i]; cout << " ";
					}

					if (i % 6 == 0) { cout << "\n"; cout << "-----------------------"; cout << "\n"; }
					if (i % 6 == 0) { Colm++; }
					if (Colm == 5) { Buffer++; Colm = 0; }
					if (Buffer == 5) { Buffer = 0; }
					if (i % 6 == 0) {
						send(Connection, "RBGDone", sizeof(buffer), NULL); //Send buffer
						send(Connection, eggchar, sizeof(buffer), NULL); //Send buffer

					}
				}
			}
		}
	}
}

void main() {

	//-------------Thermal Cam---------------------
	src = imread("C:\\Users\\Midtnight\\Desktop\\Thermal\\test2-30min\\Thermal1.jpg");
	gridThermal(); src = imread("C:\\Users\\Midtnight\\Desktop\\test.jpg");
	midPointThermal();

	//-----------------RGB-------------------------
	RGB = imread("C:\\Users\\Midtnight\\Desktop\\RGB\\test2-30min\\RGB1.jpg");
	gridRGB(); RGB = imread("C:\\Users\\Midtnight\\Desktop\\test3.jpg");
	src = RGB;
	midPointRGB();

	system("start C:\\Users\\Midtnight\\Downloads\\Master.exe");
	system("start C:\\Users\\Midtnight\\Downloads\\Robot-Client.exe");
	system("start C:\\Users\\Midtnight\\Downloads\\Conveyor.exe");

	//Winsock Startup
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0)
	{
		MessageBoxA(NULL, "Winsock startup failed", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	SOCKADDR_IN addr; //Address to be binded to our Connection socket
	int sizeofaddr = sizeof(addr); //Need sizeofaddr for the connect function
	//addr.sin_addr.s_addr = inet_addr("172.17.168.237"); //Address = localhost (this pc)
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //Address = localhost (this pc)
	addr.sin_port = htons(11111); //Port = 1111
	addr.sin_family = AF_INET; //IPv4 Socket

	Connection = socket(AF_INET, SOCK_STREAM, NULL); //Set Connection socket
	if (connect(Connection, (SOCKADDR*)&addr, sizeofaddr) != 0) //If we are unable to connect...
	{
		MessageBoxA(NULL, "Failed to Connect", "Error", MB_OK | MB_ICONERROR);
		return; //Failed to Connect
	}

	std::cout << "Connected!" << std::endl;
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientThread, NULL, NULL, NULL);
	//Create the client thread that will receive any data that the server sends.


	char buffer[256] = ""; //256 char buffer to send message

	send(Connection, "RBG Camera#10101", sizeof(buffer), NULL); //Send buffer

	while (true)
	{
		//send(Connection, buffer, sizeof(buffer), NULL); //Send buffer
		Sleep(100);
	}
	return;
}