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

#define WIDTH 160
#define HEIGHT 120

using namespace cv;
using namespace std;
//int img_size = 57600;
int img_size = 3*WIDTH*HEIGHT;
#define BUFLEN 1024
bool written = false;
char* text = "";


bool encode = false;
bool resizeVideo = false;

int la = 6;
int qp = 5; 
int bframes = 5;
int keyint = 0;
int minkeyint = 0;

int bit_to_change = 0;



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
	exit(0);
}



void serverYUV(){

	//init socket
	int rc = 0;

	uint32_t recv_size;
	uint8_t * img = (uint8_t*)malloc(img_size*sizeof(uint8_t));
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
	int frame_width = WIDTH;
	int frame_height = HEIGHT;
	x265Decoder decoder;
	decoder.initDecoder(frame_width, frame_height);




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


		
		Mat* decodedFrame = new Mat(frame_height, frame_width, CV_8UC3);
		Mat* decodedTextFrame = new Mat(frame_height, frame_width, CV_8UC3);
		bool decoded = false;


		if (encode){
			x265_nal *pp_nal = new x265_nal(); /*(x265_nal*)malloc(sizeof(pp_nal));*/
			pp_nal->sizeBytes = (int)recv_size;
			pp_nal->payload = img;
			decoder.decodeFrame(pp_nal, decodedFrame, decodedTextFrame, &decoded);
		}
		else{
			decodedFrame->data = img;
		}





		if (decoded || !encode){
			char* decodedText;
			if (encode)
				decodedText = imgDestegaMat(decodedTextFrame, false, bit_to_change);
			else
				decodedText = imgDestegaMat(decodedFrame, true, bit_to_change);
			if (strlen(decodedText) > 0){
				HANDLE hConsole;
				hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(hConsole, 11);
				cout << "Partner: " << decodedText << endl;
				SetConsoleTextAttribute(hConsole, 7);
			}
		}
		
		Mat frame(HEIGHT, WIDTH, CV_8UC3);

		if (resizeVideo){
			cv::resize(*decodedFrame, frame, Size(640, 480), 0, 0, INTER_CUBIC);
			imshow("DecodeVideo", frame);
		}
		else{
			imshow("DecodeVideo", *decodedFrame);	
		}
		if (waitKey(1) == 27){
			break;
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
	std::exit(0);
}


void captureToYuv(){

	//Launch webcam and set resolution of 160x120 for faster encoding
	OpenCVWebcam vcap;
	vcap.setWidth(WIDTH);
	vcap.setHeight(HEIGHT);
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
	x265Encoder encoder(la, qp, bframes, keyint, minkeyint);
	encoder.initEncoder(frame_width, frame_height);
	x265_nal *pp_nal;
	uint32_t pi_nal;


	thread t3(askText);
	//	t3.join();

	while (1){
		Mat readIn;
		readIn = vcap.capture(); // read a new frame from video


		//convert frame to YUV
		//Mat frame = readIn.clone();

		Mat frame(HEIGHT, WIDTH, CV_8UC3);

		

		
		cv::resize(readIn, frame, Size(WIDTH,HEIGHT), 0, 0, INTER_CUBIC);

		if (encode){
			cvtColor(frame, frame, CV_BGR2YUV_I420);
		}
		if (written){
			HANDLE hConsole;
			hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, 12);
			cout << "Me: " << text << endl;
			SetConsoleTextAttribute(hConsole, 7);
			imgStegaMat(&frame, text, bit_to_change);

			written = false;
		}/*
		else{
		imgStegaMat(&frame, "   ");
		}*/


		int size_send = 0;
		if (encode){
			img_size = (frame.dataend - frame.datastart);
		}
		else{
			size_send = frame.total()*frame.elemSize();
		}
		


		//Encode a frame using the x265_encoder

		if (encode){
			encoder.encodeFrame(&frame);

			pp_nal = encoder.get_ppnal();
			pi_nal = encoder.get_pinal();

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
			rc = zmq_send(socket, (uchar*)frame.data, size_send, 0);
			
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
	std::exit(0);
}

void check_error(int val) {
	if (val < 0){
		fprintf(stderr, "Error load YUV file!\nPress ENTER to exit\n");
		getchar();
		std::exit(-1);
	}
}


void decodeFromText(char* fileName){


	Mat testFrame(HEIGHT, WIDTH, CV_16SC3);
	std::ifstream inFile(fileName);
	for (int i = 0; i < (testFrame.dataend - testFrame.datastart) / sizeof(uchar); i++){
		inFile >> testFrame.data[i];
	}

	std::cout << "Decoded Text: " << imgDestegaMat(&testFrame, false, bit_to_change) << endl;

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

	if (argc < 2){
		std::cout << "Usage: VideoChat.exe <Encode: true or false> [<resize video true or fasle> <lookahead buffers> <qp> <bframes> <keyint> <minkeyint> <Bit to change>]" << endl;
		getchar();
		exit(0);
	}

	if ((argc > 2 && argc < 9)||(argc > 9)){
		std::cout << "Usage: VideoChat.exe <Encode: true or false> [<resize video true or fasle> <lookahead buffers> <qp> <bframes> <keyint> <minkeyint>]" << endl;
		getchar();
		exit(0);
	}
	else if (argc == 2){
		encode = (0==strcmp("true", argv[1]));
	}
	else{
		encode = (0==strcmp("true", argv[1]));

		resizeVideo = (0==strcmp("true", argv[2]));

		la = std::stoi(argv[3]);

		qp = std::stoi(argv[4]);

		bframes = std::stoi(argv[5]);

		keyint = std::stoi(argv[6]);

		minkeyint = std::stoi(argv[7]);

		char* changebit = argv[8];
		bit_to_change = std::stoi(argv[8]);
	}
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
}
