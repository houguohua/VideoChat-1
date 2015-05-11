#include <stdio.h>
#include <stdlib.h>
#include "SteganoRaw.h"
#include "encoder.h"
#include "yuv.h"
#include "param.h"
#include "x265Encoder.h"


x265Encoder::x265Encoder(){
	/* x265_param_alloc:
	*  Allocates an x265_param instance. The returned param structure is not
	*  special in any way, but using this method together with x265_param_free()
	*  and x265_param_parse() to set values by name allows the application to treat
	*  x265_param as an opaque data struct for version safety */
	x265_param *param = x265_param_alloc();

	/*      returns 0 on success, negative on failure (e.g. invalid preset/tune name). */
	x265_param_default_preset(param, "ultrafast", "zerolatency");

	/* x265_param_parse:
	*  set one parameter by name.
	*  returns 0 on success, or returns one of the following errors.
	*  note: BAD_VALUE occurs only if it can't even parse the value,
	*  numerical range is not checked until x265_encoder_open().
	*  value=NULL means "true" for boolean options, but is a BAD_VALUE for non-booleans. */
#define X265_PARAM_BAD_NAME  (-1)
#define X265_PARAM_BAD_VALUE (-2)
	x265_param_parse(param, "fps", "30");
	x265_param_parse(param, "input-res", "160x120"); //wxh
	x265_param_parse(param, "bframes", "3");
	x265_param_parse(param, "rc-lookahead", "5");
	x265_param_parse(param, "repeat-headers", "1");
	x265_param_parse(param, "pools", "none");

	/* x265_picture_alloc:
	*  Allocates an x265_picture instance. The returned picture structure is not
	*  special in any way, but using this method together with x265_picture_free()
	*  and x265_picture_init() allows some version safety. New picture fields will
	*  always be added to the end of x265_picture */;
	pic_in = &pic_orig;
	pic_recon = &pic_out;

	/***
	* Initialize an x265_picture structure to default values. It sets the pixel
	* depth and color space to the encoder's internal values and sets the slice
	* type to auto - so the lookahead will determine slice type.
	*/
	x265_picture_init(param, pic_in);

	/* x265_encoder_encode:
	*      encode one picture.
	*      *pi_nal is the number of NAL units outputted in pp_nal.
	*      returns negative on error, zero if no NAL units returned.
	*      the payloads of all output NALs are guaranteed to be sequential in memory. */
	pp_nal = NULL;
	pi_nal = 0;
	encoder = x265_encoder_open(param);
}


void x265Encoder::initEncoder(int width, int height){	
	frame_width = width;
	frame_height = height;
	depth = 8;
	colorSpace = X265_CSP_I420;

}

void x265Encoder::encodeFrame(cv::Mat* frame){
	uint32_t pixelbytes = depth > 8 ? 2 : 1;
	pic_orig.colorSpace = colorSpace;
	pic_orig.bitDepth = depth;
	pic_orig.stride[0] = frame_width * pixelbytes;
	pic_orig.stride[1] = pic_orig.stride[0] >> x265_cli_csps[colorSpace].width[1];
	pic_orig.stride[2] = pic_orig.stride[0] >> x265_cli_csps[colorSpace].width[2];
	pic_orig.planes[0] = frame->data;
	pic_orig.planes[1] = (char*)pic_orig.planes[0] + (pic_orig.stride[0] * frame_height);
	pic_orig.planes[2] = (char*)pic_orig.planes[1] + (pic_orig.stride[1] * (frame_height >> x265_cli_csps[colorSpace].height[1]));

	int encoded = x265_encoder_encode(encoder, &pp_nal, &pi_nal, pic_in, pic_recon);
	if (encoded > 0){
		int t = 1;
	}
}

x265_nal* x265Encoder::get_ppnal(){
	return pp_nal;
}

uint32_t x265Encoder::get_pinal(){
	return pi_nal;
}