#ifndef X265DECODER_H
#define X265DECODER_H

#include <stdio.h>
#include <stdlib.h>
#include "SteganoRaw.h"
#include "encoder.h"
#include "DecoderInterface.h"
#include "yuv.h"
#include "param.h"


using namespace std;
using namespace cv;

// Derived classes
class x265Decoder : public DecoderInterface
{
public:
	x265Decoder();

	~x265Decoder();

	void initDecoder(int width, int height);

	void decodeFrame(x265_nal *pp_nal, Mat* decodedFrame, bool* frameDecoded);

	Mat avframe_to_cvmat(AVFrame *frame);

protected:
	AVCodec *codec;
	AVCodecContext *av_codec_context;
	AVFrame *av_frame;
	AVPacket avpkt;
};

#endif // ifndef X265DECODER_H
