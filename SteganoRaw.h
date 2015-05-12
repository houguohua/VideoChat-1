/*
Steganography coder for raw bitmap's
*/


#ifndef STEGANORAW_H
#define STEGANORAW_H

#include <opencv\cv.h>
#include <opencv\highgui.h>



/* Encrypt a text into a bitmap image.
* first parameter is an image, second parameter is the message.
* An error will result a return of -1 to error or 0 on successfull encryption.
*/
int imgStega(IplImage*, char*);
int imgStegaMat(cv::Mat*, char*, int bit_to_change);

/* Decrypts a text from a bitmap image.
* An error will result a return of -1 or 0 on successfull decryption.
*/
char* imgDestega(IplImage*);
char* imgDestegaMat(cv::Mat*, bool safeExtraction, int bit_to_change);
#endif // !STEGANORAW_H