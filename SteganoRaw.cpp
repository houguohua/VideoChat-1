#include <opencv\cv.h>
#include <string>
#include <bitset>
#include <iostream>
#include "SteganoRaw.h"
#define BIT_TO_CHANGE 7

using namespace std;



unsigned char ToByte(bool b[8])
{
	unsigned char c = 0;
	for (int i = 0; i < 8; ++i)
		if (b[i])
			c |= 1 << i;
	return c;
}


int imgStegaMat(cv::Mat* img, char *msg) {
	int width = img->cols;
	int height = img->rows;
	int len = strlen(msg);

	uchar* data = (uchar*)img->data;


	//create new string with delimiters
	string new_str = "";
	new_str.append("$");
	new_str.append(msg);
	new_str.append("$");

	//convert new string to array of chars in bits
	bitset<8>* new_str_bits = (bitset<8>*) malloc(new_str.size()*sizeof(bitset<8>));
	for (std::size_t i = 0; i < new_str.size(); ++i)
	{
		new_str_bits[i] = bitset<8>(new_str.c_str()[i]);
	}

	//apply steganography
	int neededAmountOfPixels = (new_str.size() * 8) / 3;
	int index = 0;
	//i iterates over pixels
	for (int i = 0; i < neededAmountOfPixels; i++) {
		//j is color component(BGR)
		for (int j = 0; j < 3; j++) {
			//get color value
			unsigned char color = data[i * 3 + j];
			bitset<8> bs(color);

			//change bs[BIT_TO_CHANGE] to our text (MSB)
			bs[BIT_TO_CHANGE] = new_str_bits[index / 8].at(index % 8);
			data[i * 3 + j] = char(bs.to_ulong());
			index++;

		}
		//TODO: make sure the last $ fills whole pixel
		//if not, we will get a character on the decoding side that has ascii number > 36 (36 == '$')

	}

	//fill last whole pixel with zero values to get $ as char at decoder end:
	if ((new_str.size() * 8) % 3 != 0){
		for (int i = 0; i < (new_str.size() * 8) % 3; i++){
			unsigned char color = data[neededAmountOfPixels * 3 + i];
			bitset<8> bs(color);

			bs[BIT_TO_CHANGE] = 0;
			data[neededAmountOfPixels * 3 + i] = char(bs.to_ulong());
		}
	}
	//printf("\n%s\n", "steganografy passed");
	return 0;
}



char* imgDestegaMat(cv::Mat *img) {
	int width = img->cols;
	int height = img->rows;
	int length = 0;
	//firstDelimeter passed?
	bool firstDel = false;
	//secondDelimeter passed?
	bool secondDel = false;



	uchar* data = (uchar*)img->data;

	char* res = (char*)malloc(sizeof(char));
	bool tmp[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	int newIndex = 0;
	for (int i = 0; i < width*height; i++) {
		for (int j = 0; j < 3; j++) {
			unsigned char color = data[i * 3 + j];
			//convert color value to bits
			bitset<8> bs(color);
			tmp[newIndex % 8] = bs.at(BIT_TO_CHANGE);

			newIndex++;

			//convert last 8 bits to char
			if (newIndex % 8 == 0){

				//printf("number: %u\n", ToByte(tmp));
				//cout << ToByte(tmp) << endl;
				//getchar();
				if (firstDel && secondDel){
					length++;
					res = (char*)realloc(res, sizeof(char)*length);
					res[length - 1] = '\0';
					return res;
				}
				if (!firstDel && ToByte(tmp) == 36)
					firstDel = 1;
				else if (!secondDel && ToByte(tmp) == 36)
					secondDel = 1;
				else if (secondDel){
					length++;
					res = (char*)realloc(res, sizeof(char)*length);
					res[length - 1] = '\0';
					return res;
				}
				else{
					length++;
					res = (char*)realloc(res, sizeof(char)*length);
					res[length - 1] = (char)ToByte(tmp);
				}

			}

			//check if char == $
		}
	}

	return res;
}