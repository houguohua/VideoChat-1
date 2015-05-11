#include <stdio.h>
#include <stdlib.h>
#include "SteganoRaw.h"
#include "encoder.h"
#include "yuv.h"
#include "param.h"
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

AVCodecContext *av_codec_context;
AVFrame *av_frame;
AVPacket avpkt;
int frame_count;
int got_frame;

void initDecoder(int width, int height){
	AVCodec *codec;
	av_codec_context = NULL;

	/* register all the codecs */
	avcodec_register_all();
	codec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
	if (!codec) {
		fprintf(stderr, "Codec not found\n");
		exit(1);
	}

	av_codec_context = avcodec_alloc_context3(codec);
	if (!av_codec_context) {
		fprintf(stderr, "Could not allocate video codec context\n");
		exit(1);
	}

	av_init_packet(&avpkt);

	av_codec_context->width = width;
	av_codec_context->height = height;
	av_codec_context->extradata = NULL;
	av_codec_context->pix_fmt = PIX_FMT_YUV420P;
	if (codec->capabilities & CODEC_CAP_TRUNCATED)
		av_codec_context->flags |= CODEC_FLAG_TRUNCATED; /* We may send incomplete frames */
	if (codec->capabilities & CODEC_FLAG2_CHUNKS)
		av_codec_context->flags |= CODEC_FLAG2_CHUNKS;

	/* open it */
	if (avcodec_open2(av_codec_context, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		exit(1);
	}


	//AVFrame *av_frame_ = icv_alloc_picture_FFMPEG(PIX_FMT_YUV420P, 160, 120, true);
	//AVFrame *av_frame_RGB_ = icv_alloc_picture_FFMPEG(PIX_FMT_RGB24, 160, 120, true);
	av_frame = av_frame_alloc();

	char *rgb_buffer = new char[120 * 160 * 3];

	frame_count = 0;


}

void freeDecoder(){
	/* some codecs, such as MPEG, transmit the I and P frame with a
	latency of one frame. You must do the following to have a
	chance to get the last frame of the video */
	avpkt.data = NULL;
	avpkt.size = 0;
	avcodec_close(av_codec_context);
	av_free(av_codec_context);
	//av_frame_free(&frame);
}

cv::Mat avframe_to_cvmat(AVFrame *frame)
{
	AVFrame dst;
	cv::Mat m;

	memset(&dst, 0, sizeof(dst));

	int w = frame->width, h = frame->height;
	m = cv::Mat(h, w, CV_8UC3);
	dst.data[0] = (uint8_t *)m.data;
	avpicture_fill((AVPicture *)&dst, dst.data[0], PIX_FMT_BGR24, w, h);

	struct SwsContext *convert_ctx = NULL;
	enum PixelFormat src_pixfmt = (enum PixelFormat)frame->format;
	enum PixelFormat dst_pixfmt = PIX_FMT_BGR24;
	convert_ctx = sws_getContext(w, h, src_pixfmt, w, h, dst_pixfmt,
		SWS_FAST_BILINEAR, NULL, NULL, NULL);
	sws_scale(convert_ctx, frame->data, frame->linesize, 0, h,
		dst.data, dst.linesize);
	sws_freeContext(convert_ctx);

	return m;
}

void decodeFrame(x265_nal *pp_nal, uint32_t pi_nal){

	AVPacket av_packet;
	got_frame = 0;
	AVCodecParserContext *avparser = av_parser_init(AV_CODEC_ID_HEVC);
	av_init_packet(&av_packet);
	av_new_packet(&av_packet, pp_nal->sizeBytes);
	av_packet.data = (uint8_t *)pp_nal->payload;
	av_packet.size = pp_nal->sizeBytes;

	avcodec_decode_video2(av_codec_context, av_frame, &got_frame, &av_packet);



	if (got_frame){
		imshow("DecodeVideo", avframe_to_cvmat(av_frame));
	}

	/*Mat m;
	AVFrame dst;
	int w = 160;
	int h = 120;
	m = cv::Mat(h, w, CV_8UC3);
	dst.data[0] = (uint8_t *)m.data;
	avpicture_fill((AVPicture *)&dst, dst.data[0], PIX_FMT_BGR24, w, h);

	enum PixelFormat src_pixfmt = (enum PixelFormat)av_frame->format;
	enum PixelFormat dst_pixfmt = PIX_FMT_BGR24;
	SwsContext *convert_ctx = sws_getContext(w, h, src_pixfmt, w, h, dst_pixfmt, SWS_FAST_BILINEAR, NULL, NULL, NULL);

	if (convert_ctx == NULL) {
		fprintf(stderr, "Cannot initialize the conversion context!\n");
		exit(1);
	}

	sws_scale(convert_ctx, av_frame->data, av_frame->linesize, 0, h,
		dst.data, dst.linesize);
	imshow("MyVideo", m);
	waitKey(30);*/
}


static AVFrame * icv_alloc_picture_FFMPEG(int pix_fmt, int width, int height, bool alloc)
{
	AVFrame * picture;
	uint8_t * picture_buf;
	int size;

	picture = av_frame_alloc();
	if (!picture)
		return NULL;
	size = avpicture_get_size((PixelFormat)pix_fmt, width, height);
	if (alloc)
	{
		picture_buf = (uint8_t *)malloc(size);
		if (!picture_buf)
		{
			avcodec_free_frame(&picture);
			std::cout << "picture buff = NULL" << std::endl;
			return NULL;
		}
		avpicture_fill((AVPicture *)picture, picture_buf, (PixelFormat)pix_fmt, width, height);
	}
	return picture;
}

