#include <stdio.h>
#include <stdlib.h>
#include "SteganoRaw.h"
#include "encoder.h"
#include "DecoderInterface.h"


using namespace std;
using namespace cv;

//Base class

void DecoderInterface::initDecoder(int width, int height){
	frame_width = width;
	frame_height = height;
}

void DecoderInterface::decodeFrame(x265_nal *pp_nal, Mat* decodedFrame, Mat* decodedTextFrame,  bool* frameDecoded){
	//Do nothing
}