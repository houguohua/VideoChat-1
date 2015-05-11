#ifndef ENCODERINTERFACE_H
#define ENCODERINTERFACE_H

#include <stdio.h>
#include <stdlib.h>
#include "SteganoRaw.h"
#include "encoder.h"
#include "yuv.h"
#include "param.h"

using namespace cv;
using namespace std;


//Base class
class EncoderInterface{
public:
	virtual void initEncoder(int width, int height) = 0;

	virtual void encodeFrame(cv::Mat* frame) = 0;

protected:
	int frame_width;
	int frame_height;
	int depth, colorSpace;
};



#endif // !ENCODERINTERFACE_H