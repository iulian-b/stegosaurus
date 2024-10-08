#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <opencv2/highgui.hpp>
using namespace std;
bool Done = false;
char Bit;

bool isSet(char Bit, int Position) {
	Bit = Bit >> Position;
	return (Bit & 1 ? true : false);
}

void LSB (cv::Mat& Cover, ifstream& Input){
	Input.get(Bit);
	int Position = 0;
	bool isFileOver = false;
	for(int X=0;X<Cover.rows;X++) {
		for(int Y=0;Y<Cover.cols;Y++) {
			for(int Col=0;Col<3;Col++) {
				cv::Vec3b Px = Cover.at<cv::Vec3b>(cv::Point(X,Y));
				if(isSet(Bit,7-Position)) Px.val[Col] |= 1;
				else Px.val[Col] &= ~1;
				cout<<Bit;
				Cover.at<cv::Vec3b>(cv::Point(X,Y)) = Px;
				Position++;
				
				if(isFileOver&&Position==8){
				cout<<Bit;
					Done = true;
					return;
				}
				if(Position==8) {
				cout<<Bit;
				
					Position=0;
					Input.get(Bit);
					if(Input.eof()) {
				
						isFileOver = true;
						Bit = '\0';
					}
				}
			}
		}
	}
}

int main(int argc, char** argv) {
	cv::Mat Cover = cv::imread(argv[1]);
	cout<<argv[1]<<"   "<<argv[2]<<"   "<<argv[3];
	
	ifstream Input(argv[2]);
	LSB(Cover, Input);
	if(!Done) {
		cout << "Message too big or file not png.\n";
		return -5;
	}
	// system("rm secret");
	cv::imwrite(argv[3],Cover);
    return 5;
}