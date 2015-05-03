
#ifndef OPENCVWEBCAM_H
#define OPENCVWEBCAM_H

#include <opencv\cv.h>
#include <opencv\highgui.h>
#include "Webcam.h"
using namespace std;
using namespace cv;

// Derived classes
class OpenCVWebcam : public Webcam
{
public:
	OpenCVWebcam();

	void setWidth(int w);

	void setHeight(int h);

	~OpenCVWebcam();

	int getFPS();

	Mat capture();

protected:
	VideoCapture* stream1;
};



#endif // !STEGANORAW_H