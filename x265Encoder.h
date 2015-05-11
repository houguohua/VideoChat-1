#ifndef X265ENCODER_H
#define X265ENCODER_H

#include <stdio.h>
#include <stdlib.h>
#include "SteganoRaw.h"
#include "encoder.h"
#include "EncoderInterface.h"
#include "yuv.h"
#include "param.h"


using namespace std;
using namespace cv;

// Derived classes
class x265Encoder : public EncoderInterface
{
public:
	x265Encoder();

	void initEncoder(int width, int height);

	void encodeFrame(cv::Mat* frame);

	x265_nal* get_ppnal();

	uint32_t get_pinal();

protected:
	x265_picture pic_orig, pic_out;
	x265_picture *pic_in, *pic_recon;
	x265_encoder *encoder;
	x265_nal *pp_nal;
	uint32_t pi_nal;
};

#endif // ifndef X265ENCODER_H
