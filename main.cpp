#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <opencv\cv.h>
//#include <opencv\highgui.h>
#include <zmq.h>
#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <opencv/../../include/opencv2/opencv.hpp>
#include <thread>
#include <bitset>
#include "SteganoRaw.h"
#include "OpenCVWebcam.h"
#include "encoder.h"
#include "yuv.h"
#include "param.h"
#include "x265Encoder.h"
#include "x265Decoder.h"

using namespace cv;
using namespace std;
int img_size = 57600;
#define BUFLEN 1024
bool written = false;
char* text = "";

void client(){
	//initialize the socket
	int rc = 0;
	void *context = zmq_ctx_new();
	void *socket = zmq_socket(context, ZMQ_PAIR);

	std::cout << "Enter ip of server" << endl;
	string ipaddress;
	cin >> ipaddress;
	std::cout << "Server ip: " << "tcp://" + ipaddress + ":9000" << endl;
	std::cout << "Initialize socket" << endl;
	rc = zmq_connect(socket, ("tcp://" + ipaddress + ":9000").c_str()); /*127.0.0.1*/
	std::cout << "RC: " + rc << endl;


	//initialize camera
	/*CvCapture* capture = cvCreateCameraCapture(0);
	if(!capture){
	cout<<"No camera found."<<endl;
	return 1;
	}*/

	OpenCVWebcam stream1;
	stream1.setWidth(120);
	stream1.setHeight(160);


	while (1){
		Mat cameraFrame;

		cameraFrame = stream1.capture();

		/*imshow("Sending", cameraFrame);*/

		if (waitKey(1) >= 0)
			break;



		//cout << "Cols: " << (int)cameraFrame.cols <<  "\nRows: " << (int)cameraFrame.rows << "\nType: " << (int)cameraFrame.type() << endl;

		//         //   //sending frame



		uchar* img = cameraFrame.data;
		img_size = cameraFrame.total()*cameraFrame.elemSize();


		for (int i = 0; i < img_size / BUFLEN + 1; i++){
			if (i == 0){
				rc = zmq_send(socket, (img + i*BUFLEN), BUFLEN, 0);
			}
			else if ((i + 1)*BUFLEN <= img_size){
				rc = zmq_send(socket, (img + i*BUFLEN), BUFLEN, ZMQ_SNDMORE);
			}
			else{
				rc = zmq_send(socket, (img + i*BUFLEN), img_size % BUFLEN, 0);
			}

			if (rc == -1){
				std::cout << "Error while sending" << endl;
				return;
			}
		}

	}

	zmq_close(socket);
	zmq_ctx_destroy(context);
}

//void server(){
//
//	//init socket	
//	int rc = 0;
//	uchar* img = (uchar*)malloc(img_size*sizeof(uchar));
//	char buf[BUFLEN];
//	void *context = zmq_ctx_new();
//
//	void *socket = zmq_socket(context, ZMQ_PAIR);
//
//
//
//
//	cout << "Initialize the socket" << endl;
//	rc = zmq_bind(socket, "tcp://*:9000");
//	if (rc == -1){
//		cout << "Initialization failed." << endl;
//		return;
//	}
//
//
//
//	while (1){
//		//cout << "Test" << endl;
//
//		//receive chunks of data
//
//		for (int i = 0; i < img_size / BUFLEN + 1; i++){
//			if (i == 0){
//				rc = zmq_recv(socket, buf, BUFLEN, 0);
//			}
//			else if ((i + 1)*BUFLEN <= img_size){
//				rc = zmq_recv(socket, buf, BUFLEN, ZMQ_RCVMORE);
//			}
//			else{
//				rc = zmq_recv(socket, buf, (img_size % BUFLEN), 0);
//
//			}
//			if (rc == -1){
//				cout << "Error receiving image." << endl;
//				break;
//			}
//
//			if ((i + 1)*BUFLEN <= img_size){
//				img = (uchar*)realloc(img, (i + 1)*BUFLEN *sizeof(uchar));
//				memcpy(img + i*BUFLEN, buf, BUFLEN);
//			}
//			else{
//				memcpy(img + ((i)*BUFLEN), buf, img_size%BUFLEN);
//			}
//
//		}
//
//
//		Mat imageToShow = Mat::zeros(120, 160, CV_8UC3);
//		imageToShow.data = img;
//		imshow("Afbeelding", imageToShow);
//		if (waitKey(1) > 0)
//			break;
//
//
//
//		////convert data to a frame
//		//IplImage* frame= cvCreateImage(cvSize(640,480), IPL_DEPTH_8U, 1);
//		////memcpy(frame->imageData, img, IMG_SIZE);
//
//		//
//		////show frame
//		//cvNamedWindow("Received", CV_WINDOW_AUTOSIZE);
//		//cvShowImage("Received", frame);
//		//waitKey(0);
//		//if(waitKey(0) >= 0) break;
//
//
//
//
//	}
//
//	zmq_close(socket);
//	zmq_ctx_destroy(context);
//}
//

void askText(){
	string inputText;
	cout << "inputtext" << endl;
	getline(cin, inputText);
	cout << endl;
	while (inputText != "stop"){
		cout << "inputtext" << endl;
		text = new char[inputText.length()];
		std::strcpy(text, inputText.c_str());
		written = true;
		getline(cin, inputText);

	}
}


void serverYUV(){

	//init socket
	int rc = 0;
	uchar* recv_size = (uchar*)malloc(8 * sizeof(byte));
	uint8_t * img = (uint8_t*)malloc(img_size*sizeof(byte));
	char buf[BUFLEN];

	void *context = zmq_ctx_new();

	void *socket = zmq_socket(context, ZMQ_PAIR);


	std::fstream bitstreamFile;
	bitstreamFile.open("serverSide.hevc", std::fstream::binary | std::fstream::out);
	if (!bitstreamFile)
	{
		x265_log(NULL, X265_LOG_ERROR, "failed to open bitstream file <%s> for writing\n", "testout.hevc");
		return;
	}

	std::cout << "Initialize the socket" << endl;
	rc = zmq_bind(socket, "tcp://*:9000");
	if (rc == -1){
		std::cout << "Initialization failed." << endl;
		return;
	}


	int teller = 0;
	while (1){
		//cout << "Test" << endl;

		//receive chunks of data

		rc = zmq_recv(socket, buf, 8, 0);
		memcpy(&recv_size, buf, 8);

		img = (uchar*)realloc(img, ((int)recv_size)*sizeof(uchar));

		rc = zmq_recv(socket, img, (int)recv_size, 0);


		bitstreamFile.write((const char*)img, (int)recv_size);



		/*Mat imageToShow = Mat::zeros(120, 160, CV_8UC3);
		imageToShow.data = img;
		cv::imshow("Afbeelding", imageToShow);
		if (waitKey(1) > 0)
		break;
		*/

		////convert data to a frame
		//IplImage* frame= cvCreateImage(cvSize(640,480), IPL_DEPTH_8U, 1);
		////memcpy(frame->imageData, img, IMG_SIZE);

		//
		////show frame
		//cvNamedWindow("Received", CV_WINDOW_AUTOSIZE);
		//cvShowImage("Received", frame);
		//waitKey(0);
		//if(waitKey(0) >= 0) break;
	}
	bitstreamFile.close();
	zmq_close(socket);
	zmq_ctx_destroy(context);
}


void captureToYuv(){

	//Launch webcam and set resolution of 160x120 for faster encoding
	OpenCVWebcam vcap;
	vcap.setWidth(160);
	vcap.setHeight(120);
	int frame_width = vcap.getWidth();
	int frame_height = vcap.getHeight();
	int fps = vcap.getFPS();

	//initialize the socket
	int rc = 0;
	void *context = zmq_ctx_new();
	void *socket = zmq_socket(context, ZMQ_PAIR);

	//CameraReader camera = new CameraReader();

	std::cout << "Enter ip of server" << endl;
	string ipaddress;
	cin >> ipaddress;
	std::cout << "Server ip: " << "tcp://" + ipaddress + ":9000" << endl;
	std::cout << "Initialize socket" << endl;
	rc = zmq_connect(socket, ("tcp://" + ipaddress + ":9000").c_str()); /*127.0.0.1*/
	std::cout << "RC: " + rc << endl;


	/*
	* Here we init the x265_encoder with all the neccesary parameters.
	*/
	initEncoder(frame_width, frame_height);
	x265_nal *pp_nal;
	uint32_t pi_nal;

	/*
	* Here we init the x265_decoder with all the neccesary parameters.
	*/
	initDecoder(frame_width, frame_height);



	int teller = 0;
	std::fstream bitstreamFile;
	bitstreamFile.open("testout.hevc", std::fstream::binary | std::fstream::out);
	if (!bitstreamFile)
	{
		x265_log(NULL, X265_LOG_ERROR, "failed to open bitstream file <%s> for writing\n", "testout.hevc");
		return;
	}


	thread t3(askText);
//	t3.join();

	while (1){
		Mat readIn;
		readIn = vcap.capture(); // read a new frame from video


		//convert frame to YUV
		//Mat frame = readIn.clone();

		Mat frame(160, 120, CV_8UC3);


		resize(readIn, frame, Size(160, 120), 0, 0, INTER_CUBIC);

		cvtColor(frame, frame, CV_BGR2YUV_I420);

		if (written){
			cout << "Inserting in image: " << text << endl;
			imgStegaMat(&frame, text);
			written = false;
			cout << "Decoded text: " << imgDestegaMat(&frame) << endl;
		}
		//namedWindow("MyVideo", WINDOW_AUTOSIZE);
		cv::imshow("MyVideo", frame); //show the frame in "MyVideo" window

		/*testFrame.data[frame_width*frame_height/2+frame_width/2] = 255;
		testFrame.data[frame_width*frame_height / 2 + frame_width / 2+1] = 255;;
		testFrame.data[frame_width*frame_height / 2 + frame_width / 2+2] = 255;;
		testFrame.data[frame_width*frame_height / 2 + frame_width / 2+3] = 255;;*/

		/*imgStegaMat(&frame, "Dit is een test");*/

		/*std::ofstream testFile("output.yuv");
		for (int i = 0; i < (frame.dataend - frame.datastart) / sizeof(uchar); i++){
		testFile << frame.data[i];
		frame.data[i] = 0;
		}
		testFile.flush();
		testFile.close();
		*/
		img_size = (frame.dataend - frame.datastart);
		/*
		Mat testFrame(160, 120, CV_16SC3);
		std::ifstream inFile("output.yuv");
		for (int i = 0; i < (testFrame.dataend - testFrame.datastart) / sizeof(uchar); i++){
		inFile >> testFrame.data[i];
		}*/

		//cout << "Decoded Text: " << imgDestegaMat(&frame) << endl;

		//Encode a frame using the x265_encoder
		encodeFrame(&frame);

		pp_nal = get_ppnal();
		pi_nal = get_pinal();

		if (pi_nal){
			for (uint32_t i = 0; i < pi_nal; i++)
			{
				//cout << pp_nal->payload << endl;
				//				std::cout << pp_nal->sizeBytes << endl;
				bitstreamFile.write((const char*)pp_nal->payload, pp_nal->sizeBytes);
				//totalbytes += nal->sizeBytes;

				//Decode a frame using the x265_encoder
				decodeFrame(pp_nal, pi_nal);

				//cout << "Test" << endl;

				//receive chunks of data

				rc = zmq_send(socket, &pp_nal->sizeBytes, 8, 0);
				rc = zmq_send(socket, (const char*)pp_nal->payload, pp_nal->sizeBytes, 0);




				pp_nal++;


			}
		}
		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			std::cout << "esc key is pressed by user" << endl;
			vcap.~OpenCVWebcam();
			break;
		}
	}

	bitstreamFile.close();


	zmq_close(socket);
	zmq_ctx_destroy(context);
}

void check_error(int val) {
	if (val < 0){
		fprintf(stderr, "Error load YUV file!\nPress ENTER to exit\n");
		getchar();
		exit(-1);
	}
}

IplImage * cvLoadImageYUV(char * name_file, int w, int h){

	IplImage *py, *pu, *pv, *pu_big, *pv_big, *image;
	int i, temp;

	FILE * pf = fopen(name_file, "rb");
	if (pf == NULL){
		fprintf(stderr, "Error open file %s\nPress ENTER to exit\n", name_file);
		getchar();
		exit(-1);
	}


	py = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
	pu = cvCreateImage(cvSize(w / 2, h / 2), IPL_DEPTH_8U, 1);
	pv = cvCreateImage(cvSize(w / 2, h / 2), IPL_DEPTH_8U, 1);

	pu_big = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
	pv_big = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);

	image = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);

	assert(py);
	assert(pu);
	assert(pv);
	assert(pu_big);
	assert(pv_big);
	assert(image);

	// Read Y
	for (i = 0; i < w*h; i++){
		temp = fgetc(pf);
		check_error(temp);

		py->imageData[i] = (unsigned char)temp;
	}


	// Read U
	for (i = 0; i < w*h / 4; i++){
		temp = fgetc(pf);
		check_error(temp);

		pu->imageData[i] = (unsigned char)temp;
	}



	// Read V
	for (i = 0; i < w*h / 4; i++){
		temp = fgetc(pf);
		check_error(temp);

		pv->imageData[i] = (unsigned char)temp;
	}

	fclose(pf);

	cvResize(pu, pu_big, CV_INTER_LINEAR);
	cvResize(pv, pv_big, CV_INTER_LINEAR);



	cvReleaseImage(&pu);
	cvReleaseImage(&pv);



	cvMerge(py, pu_big, pv_big, NULL, image);

	cvReleaseImage(&py);
	cvReleaseImage(&pu_big);
	cvReleaseImage(&pv_big);



	return image;

}


void decodeFromText(char* fileName){

	/*Mat testFrame(cvLoadImageYUV(fileName, 160, 120));
	char* decodedText = imgDestegaMat(&testFrame);
	for (int i = 0; i < 10; i++){
	cout << "Char: " << (unsigned int)decodedText[i] << endl;
	}
	cout << "Decoded Text: " << imgDestegaMat(&testFrame) << endl;
	*/
	Mat testFrame(160, 120, CV_16SC3);
	std::ifstream inFile(fileName);
	for (int i = 0; i < (testFrame.dataend - testFrame.datastart) / sizeof(uchar); i++){
		inFile >> testFrame.data[i];
	}

	std::cout << "Decoded Text: " << imgDestegaMat(&testFrame) << endl;

	return;
}

void decodeFromFile(){
	std::cout << "Input Text: Dit is een test:" << endl;;

	decodeFromText("out.yuv");
	getchar();
	return;
}

void yuvDemoStegano(){

	captureToYuv();
}

int main(int argc, char** argv){
	//thread t2(client);
	//t2.join();
	//thread t1(server);
	//t1.join();
	//
	//yuvDemoStegano();

	thread t2(serverYUV);
	thread t1(captureToYuv);
	
	t1.join();
	t2.join();
	//decodeFromFile();


	return 0;

	//
	//
	//getchar();
	//imwrite("C:/Users/kiani/Downloads/test.jpg", matimg);
	////IplImage* img2 = cvLoadImage("C:/Users/kiani/Downloads/test.jpg");
	//printf("result: %s",imgDestega(img));
	//std::getchar();
	//captureToYuv();

}