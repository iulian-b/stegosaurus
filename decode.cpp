#include <iostream>
#include <fstream>
#include <opencv2/highgui.hpp>
using namespace std;
char Bit=0;
string Output = "";
bool isSet(char Bit, int Position) {
    Bit = Bit >> Position;
    return (Bit & 1 ? true : false);
}

void ReverseLSB(cv::Mat& Stego){
    int Position = 0;
    for(int X=0;X<Stego.rows;X++) {
        for(int Y=0;Y<Stego.cols;Y++) {
            for(int Col=0;Col<3;Col++) {
                cv::Vec3b Px = Stego.at<cv::Vec3b>(cv::Point(X,Y));
                if(isSet(Px.val[Col],0)) Bit |= 1;
                Position++;
                if(Position == 8) {
                    if(Bit == '\0') return;
                    Position=0;
                    cout<<Bit;
                    Output+=Bit;
                    Bit=0;
                }
                else Bit=Bit<<1;
            }
        }
    }
}

int main(int argc, char** argv) {
    cv::Mat Stego = cv::imread(argv[1]);
    if(Stego.empty()) {
        cout << "[ERROR] Image type not accepted"<<endl;
        return 5;
    }

    ReverseLSB(Stego);
    ofstream out("StegoSaurus-LSB-Extract");
    out<<Output;
    out.close();
    return 0;
}
