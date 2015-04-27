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
#include "encoder.h"
#include "yuv.h"
#include "param.h"


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
	//vcap.set(CV_CAP_PROP_FRAME_HEIGHT, 120);
	//vcap.set(CV_CAP_PROP_FRAME_WIDTH, 160);

	if (!vcap.isOpened()){
		cout << "Error opening video stream or file" << endl;
		return;
	}

	int frame_width = vcap.get(CV_CAP_PROP_FRAME_WIDTH);
	int frame_height = vcap.get(CV_CAP_PROP_FRAME_HEIGHT);
	int fps = vcap.get(CV_CAP_PROP_FPS);


	/* x265_param_alloc:
	*  Allocates an x265_param instance. The returned param structure is not
	*  special in any way, but using this method together with x265_param_free()
	*  and x265_param_parse() to set values by name allows the application to treat
	*  x265_param as an opaque data struct for version safety */
	x265_param *param = x265_param_alloc();

	/*      returns 0 on success, negative on failure (e.g. invalid preset/tune name). */
	x265_param_default_preset(param, "ultrafast", "zerolatency");

	/* x265_param_parse:
	*  set one parameter by name.
	*  returns 0 on success, or returns one of the following errors.
	*  note: BAD_VALUE occurs only if it can't even parse the value,
	*  numerical range is not checked until x265_encoder_open().
	*  value=NULL means "true" for boolean options, but is a BAD_VALUE for non-booleans. */
#define X265_PARAM_BAD_NAME  (-1)
#define X265_PARAM_BAD_VALUE (-2)
	x265_param_parse(param, "fps", "30");
	x265_param_parse(param, "input-res", "640x480"); //wxh
	x265_param_parse(param, "bframes", "3");
	x265_param_parse(param, "rc-lookahead", "5");
	x265_param_parse(param, "repeat-headers", "1");

	/* x265_picture_alloc:
	*  Allocates an x265_picture instance. The returned picture structure is not
	*  special in any way, but using this method together with x265_picture_free()
	*  and x265_picture_init() allows some version safety. New picture fields will
	*  always be added to the end of x265_picture */
	x265_picture pic_orig, pic_out;
	x265_picture *pic_in = &pic_orig;
	x265_picture *pic_recon = &pic_out;



	/***
	* Initialize an x265_picture structure to default values. It sets the pixel
	* depth and color space to the encoder's internal values and sets the slice
	* type to auto - so the lookahead will determine slice type.
	*/
	x265_picture_init(param, pic_in);



	/* x265_encoder_encode:
	*      encode one picture.
	*      *pi_nal is the number of NAL units outputted in pp_nal.
	*      returns negative on error, zero if no NAL units returned.
	*      the payloads of all output NALs are guaranteed to be sequential in memory. */
	x265_nal *pp_nal;
	uint32_t pi_nal;
	x265_encoder *encoder = x265_encoder_open(param);
	//x265_encoder_encode(encoder, &pp_nal, &pi_nal, pic_in, pic_out);

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

		
		
		/*testFrame.data[frame_width*frame_height/2+frame_width/2] = 255;
		testFrame.data[frame_width*frame_height / 2 + frame_width / 2+1] = 255;;
		testFrame.data[frame_width*frame_height / 2 + frame_width / 2+2] = 255;;
		testFrame.data[frame_width*frame_height / 2 + frame_width / 2+3] = 255;;*/
		int depth = 8;
		int colorSpace = X265_CSP_I420; // wat is dit? Welke waarden mogen we hier meegeven?

		std::ofstream testFile("output.yuv");
		for (int i = 0; i < (frame.dataend-frame.datastart)/sizeof(uchar); i++){
			testFile << frame.data[i];
		}
		
		


		uint32_t pixelbytes = depth > 8 ? 2 : 1;
		pic_orig.colorSpace = colorSpace;
		pic_orig.bitDepth = depth;
		pic_orig.stride[0] = frame_width * pixelbytes;
		pic_orig.stride[1] = pic_orig.stride[0] >> x265_cli_csps[colorSpace].width[1];
		pic_orig.stride[2] = pic_orig.stride[0] >> x265_cli_csps[colorSpace].width[2];
		pic_orig.planes[0] = frame.data;
		pic_orig.planes[1] = (char*)pic_orig.planes[0] + (pic_orig.stride[0] * frame_height);
		pic_orig.planes[2] = (char*)pic_orig.planes[1] + (pic_orig.stride[1] * (frame_height >> x265_cli_csps[colorSpace].height[1]));

		int encoded = x265_encoder_encode(encoder, &pp_nal, &pi_nal, pic_in, pic_recon);

		if (pi_nal){
			for (uint32_t i = 0; i < pi_nal; i++)
			{
				//cout << pp_nal->payload << endl;
				bitstreamFile.write((const char*)pp_nal->payload, pp_nal->sizeBytes);
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

	//bitstreamFile.close();
}

int main(int argc, char** argv){
	//thread t2(client);
	//t2.join();
	//thread t1(server);
	//t1.join();
	//
	captureToYuv();
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