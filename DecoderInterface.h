#ifndef DECODERINTERFACE_H
#define DECODERINTERFACE_H

#include <stdio.h>
#include <stdlib.h>


using namespace cv;
using namespace std;

//Base class
class DecoderInterface{
public:
	virtual void initDecoder(int width, int height) = 0;

	virtual void decodeFrame(x265_nal *pp_nal, Mat* decodedFrame, Mat* decodedTextFrame,  bool* frameDecoded) = 0;

protected:
	int frame_width;
	int frame_height;
	int frame_count;
	int got_frame;
};



#endif // !DECODERINTERFACE_H