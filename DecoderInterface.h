#ifndef DECODERINTERFACE_H
#define DECODERINTERFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <opencv\cv.h>
extern "C"
{
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
}

#pragma comment(lib, "avcodec.lib")

using namespace cv;
using namespace std;

//Base class
class DecoderInterface{
public:
	virtual void initDecoder(int width, int height) = 0;

	virtual void decodeFrame(x265_nal *pp_nal, Mat* decodedFrame, bool* frameDecoded) = 0;

protected:
	int frame_count;
	int got_frame;
};



#endif // !DECODERINTERFACE_H