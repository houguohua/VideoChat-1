/*
Steganography coder for raw bitmap's
*/


#ifndef WEBCAM_H
#define WEBCAM_H

#include <opencv\cv.h>
#include <opencv\highgui.h>

using namespace std;
using namespace cv;

//Base class
class Webcam{
public:
	virtual Mat capture() = 0;

	void setWidth(int w);

	void setHeight(int h);

	int getWidth();

	int getHeight();

protected:
	int width;
	int height;
};



#endif // !STEGANORAW_H