#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <opencv\cv.h>
//#include <opencv\highgui.h>
#include <zmq.h>
#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <opencv/../../include/opencv2/opencv.hpp>
#include <thread>
#include <bitset>
#include "OpenCVWebcam.h"
#include "encoder.h"
#include "yuv.h"
#include "param.h"

OpenCVWebcam::OpenCVWebcam()
	{
		stream1 = new VideoCapture(0);
		stream1->set(CV_CAP_PROP_CONVERT_RGB, false);
	}

	void OpenCVWebcam::setWidth(int w){
		width = w;
		stream1->set(CV_CAP_PROP_FRAME_WIDTH, w);
	}

	void OpenCVWebcam::setHeight(int h){
		height = h;
		stream1->set(CV_CAP_PROP_FRAME_HEIGHT, h);
	}

	int OpenCVWebcam::getFPS(){
		return stream1->get(CV_CAP_PROP_FPS);
	}

	OpenCVWebcam::~OpenCVWebcam(){
		stream1->release();
	}

	Mat OpenCVWebcam::capture(){
		Mat frame;
		stream1->read(frame);
		return frame;
	}
