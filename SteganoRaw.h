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
int imgStegaMat(cv::Mat*, char*);

/* Decrypts a text from a bitmap image.
* An error will result a return of -1 or 0 on successfull decryption.
*/
char* imgDestega(IplImage*);
char* imgDestegaMat(cv::Mat*);
#endif // !STEGANORAW_H