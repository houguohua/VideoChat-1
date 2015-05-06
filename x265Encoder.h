#ifndef X265ENCODER_H
#define X265ENCODER_H

#include <stdio.h>
#include <stdlib.h>
#include "SteganoRaw.h"
#include "encoder.h"
#include "yuv.h"
#include "param.h"

using namespace std;
using namespace cv;

void initEncoder(int width, int height);
void encodeFrame(cv::Mat* frame);
x265_nal* get_ppnal();
uint32_t get_pinal();

#endif // ifndef X265ENCODER_H
