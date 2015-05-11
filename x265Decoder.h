#ifndef X265DECODER_H
#define X265DECODER_H

#include <stdio.h>
#include <stdlib.h>
#include "SteganoRaw.h"
#include "encoder.h"
#include "yuv.h"
#include "param.h"

using namespace std;
using namespace cv;

void initDecoder(int width, int height);
void freeDecoder();
void decodeFrame(x265_nal *pp_nal, uint32_t pi_nal);

#endif // ifndef X265DECODER_H
