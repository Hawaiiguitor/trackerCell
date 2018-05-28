#include<opencv2\opencv.hpp>
#include <iostream>
#include <opencv2\video\tracking.hpp>
#include <stdlib.h>
#include <opencv2/core/utility.hpp>
#include <fstream>

using namespace cv;
using namespace std;

bool First_Freme_flag = false;  //第一帧图像用于设定ROI
bool drawing_box = false;
bool destroy = false;
Rect box;
Point point;
Mat cap_img;
int drag = 0;
Mat frame;

const char *winName = "RT4 Tracking";

void draw_box(Mat &src, Rect box);
void mouse_callback_ExtractROI(int event, int x, int y, int flags, void *param);

int main()
{
	//Setting the simpleBlobDector parameter to detect the object;
	SimpleBlobDetector::Params DetectorPara;
	//DetectorPara.filterByColor
//	DetectorPara.minThreshold = 0;
//	DetectorPara.maxThreshold = 30;
	DetectorPara.filterByCircularity = false;
	DetectorPara.filterByColor = true;
	DetectorPara.blobColor = 0;
	DetectorPara.filterByInertia = false;
	DetectorPara.filterByConvexity = false;
	DetectorPara.filterByArea = true;
	DetectorPara.minArea = 50;
	DetectorPara.maxArea = 9000;
	//DectcorParam.minConvexity = 0.1;
	//DectcorParam.maxConvexity = 1;
	Ptr<SimpleBlobDetector> blobObject = SimpleBlobDetector::create(DetectorPara);
	

	vector<KeyPoint> keypoints; // storage for blobs

	const string path = "RT4_120_5.avi";
	VideoCapture cap(path);
	if (!cap.isOpened()) {
		cout << "Could not open reference " << endl;
		return -1;
	}
	namedWindow(winName, WINDOW_AUTOSIZE);
	int frameNum = 0;

	cap >> frame;
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	imshow(winName, frame);
	waitKey(10);

	//select roi manually 
	while (0 == First_Freme_flag) {
		cvSetMouseCallback(winName, mouse_callback_ExtractROI, NULL);
		waitKey(10);
	}

	for (;;) {
		if (First_Freme_flag == 1) {
			char c;
			Mat temp, temp_gray, binaryImg;
			cap >> frame;
			if (frame.empty()) {
				cout << " < < < Game over! > > >";
				break;
			}
			//	frame.resize(640, 480);
			++frameNum;
			

			temp = frame(box);

			cvtColor(temp, temp_gray, CV_RGB2GRAY);
			//imshow(winName, temp_gray);

			GaussianBlur(temp_gray, temp_gray, Size(7, 7), 3, 3);
			threshold(temp_gray, binaryImg, 0, 255, THRESH_BINARY+THRESH_OTSU);
			blobObject->detect(binaryImg, keypoints);

		//	waitKey(10);
			// Draw detected blobs as red circles. 
		    // DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures 
			// the size of the circle corresponds to the size of blob
			Mat im_with_keypoints;
			drawKeypoints(binaryImg, keypoints, im_with_keypoints, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
			
			cout << "Frame: " << frameNum << "# ";
			if (keypoints.size() == 0) {

				cout << "The object was not detected." << endl;
				return -1;
			}
			else {
				cout << "x position: " << keypoints[0].pt.x << endl;
			}
			
			// put the x position into "Tracking.txt" 
			ofstream fout; //create an ofstream object named fout
			fout.open("Tracking.txt", ios::out | ios::app);  //associate fout with Tracking.txt
			if (!fout.is_open()) // open attempt failed
			{
				cout << "cann't open this file. " << endl;
				return -1;
			}
			fout << "Frame: "<< frameNum << "	x position: "<< keypoints[0].pt.x << endl; // the content, existing in the file, will be overrided.
			//fout.close(); //close output connection to file



			//rectangle(frame, box, Scalar(255, 128, 128), 2);
			imshow(winName, im_with_keypoints);
			//c = (char)waitKey(10);
			//temp = frame(box);
			//imshow("RT4 video", temp);
			c = (char)waitKey(27);
			if (c == 27) break;
	
		}
	}
	return 0;
}

void draw_box(Mat &src, Rect rect)
{
	rectangle(src, Point(box.x, box.y), Point(box.x + box.width, box.y + box.height), Scalar(0, 0, 255), 2);
	//imshow("Result", srcImg);
	Rect rect2 = Rect(Point(box.x, box.y), Point(box.x + box.width, box.y + box.height));
	First_Freme_flag = true;
}

void mouse_callback_ExtractROI(int event, int x, int y, int flags, void *param)
{
	Mat temp;
	static Point pre_pt = (-1, -1);//初始坐标  
	static Point cur_pt = (-1, -1);//实时坐标 
	switch (event)
	{
	case CV_EVENT_MOUSEMOVE:
	{
		Mat temp;
		if (drawing_box)
		{
			frame.copyTo(temp);
			box.width = x - box.x;
			box.height = y - box.y;
			//temp = frame(box);
			cur_pt = Point(x, y);
			rectangle(temp, pre_pt, cur_pt, Scalar(255, 0, 0), 2, 8, 0);
			imshow(winName, temp);
		}
	}
	break;

	case CV_EVENT_LBUTTONDOWN:
	{
		pre_pt = Point(x, y);
		drawing_box = true;
		box = Rect(x, y, 0, 0);
	}
	break;

	case CV_EVENT_LBUTTONUP:
	{
		drawing_box = false;
		if (box.width < 0)
		{
			box.x += box.width;
			box.width *= -1;
		}

		if (box.height < 0)
		{
			box.y += box.height;
			box.height *= -1;
		}

		draw_box(frame, box);
	}
	break;

	case CV_EVENT_RBUTTONUP:
	{
		destroy = true;
	}
	break;

	default:
		break;
	}
}