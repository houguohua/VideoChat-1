#include "x265Decoder.h"

using namespace cv;
using namespace std;

x265Decoder::x265Decoder(){
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
}


void x265Decoder::initDecoder(int width, int height){

	frame_width = width;
	frame_height = height;

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

	av_frame = av_frame_alloc();

	char *rgb_buffer = new char[120 * 160 * 3];

	frame_count = 0;

}

x265Decoder::~x265Decoder(){
	/* some codecs, such as MPEG, transmit the I and P frame with a
	latency of one frame. You must do the following to have a
	chance to get the last frame of the video */
	avpkt.data = NULL;
	avpkt.size = 0;
	avcodec_close(av_codec_context);
	av_free(av_codec_context);
}

Mat x265Decoder::avframe_to_cvmat(AVFrame *frame, bool text)
{
	AVFrame dst;
	cv::Mat m;

	memset(&dst, 0, sizeof(dst));

	int w = frame->width, h = frame->height;
	m = cv::Mat(h, w, CV_8UC3);
	dst.data[0] = (uint8_t *)m.data;
	if (text){
		avpicture_fill((AVPicture *)&dst, dst.data[0], PIX_FMT_YUV420P, w, h);
	}
	else{
		avpicture_fill((AVPicture *)&dst, dst.data[0], PIX_FMT_BGR24, w, h);
	}


	struct SwsContext *convert_ctx = NULL;
	enum PixelFormat src_pixfmt = (enum PixelFormat)frame->format;
	enum PixelFormat dst_pixfmt;
	if (text){
		dst_pixfmt = PIX_FMT_YUV420P;
	}
	else{
		dst_pixfmt = PIX_FMT_BGR24;
	}

	convert_ctx = sws_getContext(w, h, src_pixfmt, w, h, dst_pixfmt,
		SWS_FAST_BILINEAR, NULL, NULL, NULL);
	sws_scale(convert_ctx, frame->data, frame->linesize, 0, h,
		dst.data, dst.linesize);
	sws_freeContext(convert_ctx);

	return m;
}

void x265Decoder::decodeFrame(x265_nal *pp_nal, Mat* decodedFrame, Mat* decodedTextFrame, bool* frameDecoded){

	AVPacket av_packet;
	got_frame = 0;
	AVCodecParserContext *avparser = av_parser_init(AV_CODEC_ID_HEVC);
	av_init_packet(&av_packet);
	av_new_packet(&av_packet, pp_nal->sizeBytes);
	av_packet.data = (uint8_t *)pp_nal->payload;
	av_packet.size = pp_nal->sizeBytes;

	avcodec_decode_video2(av_codec_context, av_frame, &got_frame, &av_packet);



	if (got_frame){
		*decodedFrame = avframe_to_cvmat(av_frame, false);
		*decodedTextFrame = avframe_to_cvmat(av_frame, true);
		*frameDecoded = true;
	}
	else{
		*frameDecoded = false;
	}
}


