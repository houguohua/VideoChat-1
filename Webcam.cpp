#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <opencv\cv.h>
#include <zmq.h>
#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <bitset>
#include "Webcam.h"
#include "encoder.h"
#include "yuv.h"
#include "param.h"


using namespace std;
using namespace cv;

//Base class

void Webcam::setWidth(int w){
	width = w;
}

void Webcam::setHeight(int h){
	height = h;
}

int Webcam::getWidth(){
	return width;
}

int Webcam::getHeight(){
	return height;
}