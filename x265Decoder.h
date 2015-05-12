#ifndef X265DECODER_H
#define X265DECODER_H

#include "encoder.h"
#include "SteganoRaw.h"
#include "DecoderInterface.h"

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


using namespace std;
using namespace cv;

// Derived classes
class x265Decoder : public DecoderInterface
{
public:
	x265Decoder();

	~x265Decoder();

	void initDecoder(int width, int height);

	void decodeFrame(x265_nal *pp_nal, Mat* decodedFrame, Mat* decodedTextFrame,  bool* frameDecoded);

	Mat avframe_to_cvmat(AVFrame *frame, bool text);

protected:
	AVCodec *codec;
	AVCodecContext *av_codec_context;
	AVFrame *av_frame;
	AVPacket avpkt;
};

#endif // ifndef X265DECODER_H
