#include <stdio.h>
#include <stdlib.h>
#include "SteganoRaw.h"
#include "encoder.h"
#include "yuv.h"
#include "param.h"
#include "EncoderInterface.h"


using namespace std;
using namespace cv;

//Base class

void EncoderInterface::initEncoder(int width, int height){
	frame_width = width;
	frame_height = height;
}

void EncoderInterface::encodeFrame(cv::Mat* frame){
	//Do nothing
}