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

bool encode = false;



void askText(){
	string inputText;
	getline(cin, inputText);
	cout << endl;
	while (inputText != "stop"){
		text = new char[inputText.length()];
		std::strcpy(text, inputText.c_str());
		written = true;
		getline(cin, inputText);

	}
}



void serverYUV(){

	//init socket
	int rc = 0;
	
	uint32_t recv_size;
	uint8_t * img = (uint8_t*)malloc(img_size*sizeof(byte));
	char buf[BUFLEN];

	void *context = zmq_ctx_new();

	void *socket = zmq_socket(context, ZMQ_PAIR);


	/*std::fstream bitstreamFile;
	bitstreamFile.open("serverSide.hevc", std::fstream::binary | std::fstream::out);
	if (!bitstreamFile)
	{
		x265_log(NULL, X265_LOG_ERROR, "failed to open bitstream file <%s> for writing\n", "testout.hevc");
		return;
	}*/

	std::cout << "Initialize the socket" << endl;
	rc = zmq_bind(socket, "tcp://*:9000");
	if (rc == -1){
		std::cout << "Initialization failed." << endl;
		return;
	}

	/*
	* Here we init the x265_decoder with all the neccesary parameters.
	*/
	int frame_width = 160;
	int frame_height = 120;
	initDecoder(frame_width, frame_height);
	


	
	while (1){
		//cout << "Test" << endl;

		//receive chunks of data

		if (encode){
			rc = zmq_recv(socket, &recv_size, 4, 0);

			img = (uchar*)realloc(img, recv_size*sizeof(uchar));

			rc = zmq_recv(socket, img, recv_size, 0);

		}
		else{
			rc = zmq_recv(socket, img, img_size, 0);
		}
		
		

		Mat* decodedFrame = new Mat(120, 160, CV_8UC3);
		bool decoded = false;

		if (encode){
			x265_nal *pp_nal = new x265_nal(); /*(x265_nal*)malloc(sizeof(pp_nal));*/
			pp_nal->sizeBytes = (int)recv_size;
			pp_nal->payload = img;
			decodeFrame(pp_nal, decodedFrame, &decoded);
		}
		

		

		if (decoded){
			char* decodedText = imgDestegaMat(decodedFrame);
			if (strlen(decodedText)>0){
				HANDLE hConsole;
				hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(hConsole, 11);
				cout << "Partner: " << decodedText << endl;
			}
			
			imshow("DecodeVideo", *decodedFrame);
			if (waitKey(1) > 0){
				break;
			}

		}
		

		
		//bitstreamFile.write((const char*)img, (int)recv_size);



		/*Mat imageToShow = Mat::zeros(120, 160, CV_8UC3);
		imageToShow.data = img;
		cv::imshow("Afbeelding", imageToShow);
		if (waitKey(1) > 0)
		break;
		*/

	}
	//bitstreamFile.close();
	/*free(recv_size);
	free(img);*/
	zmq_close(socket);
	zmq_ctx_destroy(context);
	exit(0);
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
			HANDLE hConsole;
			hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, 12);
			cout << "Me: " << text << endl;
			imgStegaMat(&frame, text);
			written = false;
		}/*
		else{
			imgStegaMat(&frame, "   ");
		}*/

		
		
		
		img_size = (frame.dataend - frame.datastart);
		

		//Encode a frame using the x265_encoder
		if (encode){
			encodeFrame(&frame);

			pp_nal = get_ppnal();
			pi_nal = get_pinal();

			if (pi_nal){
				for (uint32_t i = 0; i < pi_nal; i++)
				{
					//bitstreamFile.write((const char*)pp_nal->payload, pp_nal->sizeBytes);
					//receive chunks of data


					rc = zmq_send(socket, &pp_nal->sizeBytes, 4, 0);
					rc = zmq_send(socket, (const char*)pp_nal->payload, pp_nal->sizeBytes, 0);




					pp_nal++;


				}
			}
		}
		else{
			rc = zmq_send(socket, (const char*)frame.data, img_size , 0);
		}
		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			std::cout << "esc key is pressed by user" << endl;
			vcap.~OpenCVWebcam();
			break;
		}
	}

	//bitstreamFile.close();


	zmq_close(socket);
	zmq_ctx_destroy(context);
	exit(0);
}

void check_error(int val) {
	if (val < 0){
		fprintf(stderr, "Error load YUV file!\nPress ENTER to exit\n");
		getchar();
		exit(-1);
	}
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
	//if (argc > )
	//if (i + 1 != argc) // Check that we haven't finished parsing already
	//	if (argv[i] == "-f") {
	//		// We know the next argument *should* be the filename:
	//		myFile = argv[i + 1];
	//	}
	//	else if (argv[i] == "-p") {
	//		myPath = argv[i + 1];
	//	}
	//	else if (argv[i] == "-o") {
	//		myOutPath = argv[i + 1];
	//	}
	//	else {
	//		std::cout << "Not enough or invalid arguments, please try again.\n";
	//		Sleep(2000);
	//		/*
	//		*  Sleep for 2 seconds to allow user (if any) to read above statement.
	//		*  The issue with this is that if we're a backend program to a GUI as mentioned above;
	//		*  that program would also sleep for 2 seconds. Most programs don't
	//		*  have this - the console will keep the text there until it scrolls off the page or whatever, so you may aswell leave it out.
	//		***/
	//		exit(0);
	//	}

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
