#include <iostream>
#include <opencv\cv.h>
//#include <opencv\highgui.h>
#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <opencv/../../include/opencv2/opencv.hpp>
#include <thread>
#include <bitset>
#include "SteganoRaw.h"
#include "encoder.h"
#include "yuv.h"
#include "param.h"
#include "x265Encoder.h"
#include "x265Decoder.h"

using namespace cv;
using namespace std;
int img_size = 57600;
#define BUFLEN 1024

void client(){
	//initialize the socket
	int rc = 0;
	void *context = zmq_ctx_new();
	void *socket = zmq_socket(context, ZMQ_PAIR);

	//CameraReader camera = new CameraReader();

	cout << "Enter ip of server" << endl;
	string ipaddress;
	cin >> ipaddress;
	cout << "Server ip: " << "tcp://" + ipaddress + ":9000" << endl;
	cout << "Initialize socket" << endl;
	rc = zmq_connect(socket, ("tcp://" + ipaddress + ":9000").c_str()); /*127.0.0.1*/
	cout << "RC: " + rc << endl;


	//initialize camera
	/*CvCapture* capture = cvCreateCameraCapture(0);
	if(!capture){
	cout<<"No camera found."<<endl;
	return 1;
	}*/

	VideoCapture stream1(0);
	stream1.set(CV_CAP_PROP_CONVERT_RGB, false);
	stream1.set(CV_CAP_PROP_FRAME_HEIGHT, 120);
	stream1.set(CV_CAP_PROP_FRAME_WIDTH, 160);


	while (1){
		Mat cameraFrame;

		stream1.read(cameraFrame);

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
				cout << "Error while sending" << endl;
				return;
			}
		}

	}

	zmq_close(socket);
	zmq_ctx_destroy(context);
}

void server(){

	//init socket	
	int rc = 0;
	uchar* img = (uchar*)malloc(img_size*sizeof(uchar));
	char buf[BUFLEN];
	void *context = zmq_ctx_new();

	void *socket = zmq_socket(context, ZMQ_PAIR);




	cout << "Initialize the socket" << endl;
	rc = zmq_bind(socket, "tcp://*:9000");
	if (rc == -1){
		cout << "Initialization failed." << endl;
		return;
	}



	while (1){
		//cout << "Test" << endl;

		//receive chunks of data

		for (int i = 0; i < img_size / BUFLEN + 1; i++){
			if (i == 0){
				rc = zmq_recv(socket, buf, BUFLEN, 0);
			}
			else if ((i + 1)*BUFLEN <= img_size){
				rc = zmq_recv(socket, buf, BUFLEN, ZMQ_RCVMORE);
			}
			else{
				rc = zmq_recv(socket, buf, (img_size % BUFLEN), 0);

			}
			if (rc == -1){
				cout << "Error receiving image." << endl;
				break;
			}

			if ((i + 1)*BUFLEN <= img_size){
				img = (uchar*)realloc(img, (i + 1)*BUFLEN *sizeof(uchar));
				memcpy(img + i*BUFLEN, buf, BUFLEN);
			}
			else{
				memcpy(img + ((i)*BUFLEN), buf, img_size%BUFLEN);
			}

		}


		Mat imageToShow = Mat::zeros(120, 160, CV_8UC3);
		imageToShow.data = img;
		imshow("Afbeelding", imageToShow);
		if (waitKey(1)>0)
			break;



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

	zmq_close(socket);
	zmq_ctx_destroy(context);
}




void captureToYuv(){

	VideoCapture vcap(0);
	//vcap.set(CV_CAP_PROP_CONVERT_RGB, false);

	//vcap.set(CV_CAP_PROP_FOURCC, CV_FOURCC('I', 'M', 'C', '3'));
	vcap.set(CV_CAP_PROP_FRAME_HEIGHT, 120);
	vcap.set(CV_CAP_PROP_FRAME_WIDTH, 160);

	if (!vcap.isOpened()){
		cout << "Error opening video stream or file" << endl;
		return;
	}

	int frame_width = vcap.get(CV_CAP_PROP_FRAME_WIDTH);
	int frame_height = vcap.get(CV_CAP_PROP_FRAME_HEIGHT);
	int fps = vcap.get(CV_CAP_PROP_FPS);

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


	std::fstream bitstreamFile;
	bitstreamFile.open("testout.hevc", std::fstream::binary | std::fstream::out);
	if (!bitstreamFile)
	{
		x265_log(NULL, X265_LOG_ERROR, "failed to open bitstream file <%s> for writing\n", "testout.hevc");
		return;
	}

	while (1){

		Mat readIn;

		bool bSuccess = vcap.read(readIn); // read a new frame from video

		Mat frame = readIn.clone();
		cvtColor(readIn, frame, CV_BGR2YUV_I420);

		if (!bSuccess) //if not success, break loop
		{
			cout << "ERROR: Cannot read a frame from video file" << endl;

		}

		imshow("MyVideo", frame); //show the frame in "MyVideo" window


		imgStegaMat(&frame, "Dit is een test");

		cout << "Decoded Text: " << imgDestegaMat(&frame) << endl;

		std::ofstream testFile("output.yuv");
		for (int i = 0; i < (frame.dataend - frame.datastart) / sizeof(uchar); i++){
			testFile << frame.data[i];
		}

		//Encode a frame using the x265_encoder
		encodeFrame(&frame);

		pp_nal = get_ppnal();
		pi_nal = get_pinal();

		if (pi_nal){
			for (uint32_t i = 0; i < pi_nal; i++)
			{
				//cout << pp_nal->payload << endl;
				bitstreamFile.write((const char*)pp_nal->payload, pp_nal->sizeBytes);
				decodeFrame(pp_nal, pi_nal);
				//totalbytes += nal->sizeBytes;
				pp_nal++;
			}
		}
		if (waitKey(10) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;

		}

	}

	bitstreamFile.close();
}

void check_error(int val) {
	if (val<0){
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
	for (i = 0; i<w*h; i++){
		temp = fgetc(pf);
		check_error(temp);

		py->imageData[i] = (unsigned char)temp;
	}


	// Read U
	for (i = 0; i<w*h / 4; i++){
		temp = fgetc(pf);
		check_error(temp);

		pu->imageData[i] = (unsigned char)temp;
	}



	// Read V
	for (i = 0; i<w*h / 4; i++){
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

	Mat testFrame(cvLoadImageYUV(fileName, 160, 120));

	char* decodedText = imgDestegaMat(&testFrame);

	for (int i = 0; i < 10; i++){
		cout << "Char: " << (unsigned int)decodedText[i] << endl;
	}


	cout << "Decoded Text: " << imgDestegaMat(&testFrame) << endl;


	return;
}

void decodeFromFile(){
	cout << "Dit is een tekst:" << endl;;
	cout << (int)'D' << endl;
	cout << (int)'i' << endl;
	cout << (int)'t' << endl;
	cout << (int)' ' << endl;
	cout << (int)'I' << endl;
	cout << (int)'s' << endl;

	decodeFromText("out.yuv");
	getchar();
	return;
}
int main(int argc, char** argv){
	//thread t2(client);
	//t2.join();
	//thread t1(server);
	//t1.join();
	//
	captureToYuv();
	//decodeFromFile();


	return 0;
	//Mat matimg = imread("C:/Users/kiani/Downloads/fruit.jpg");
	//string input;
	//getline(cin, input);
	//while (input != "stop"){
	//	char* toEncode = (char*) input.c_str();
	//	printf("%-15s %s\n", "Encoding:", toEncode);
	//	imgStegaMat(&matimg, toEncode);

	//	printf("%-15s %s\n", "Result decoder:", imgDestegaMat(&matimg));
	//	getline(cin, input);
	//	printf("\n\n");
	//}
	//
	//
	//getchar();
	//imwrite("C:/Users/kiani/Downloads/test.jpg", matimg);
	////IplImage* img2 = cvLoadImage("C:/Users/kiani/Downloads/test.jpg");
	//printf("result: %s",imgDestega(img));
	//std::getchar();
	//captureToYuv();

}