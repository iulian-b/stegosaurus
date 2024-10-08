#include <iostream>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
using namespace std;

void Slice(cv::Mat plane, cv::Mat output, int bit){
    int col = plane.cols;
    int row = plane.rows;

    for(int y=0; y<row; y++){
        for(int x=0; x<col; x++){
            output.at<uchar>(y,x) = (plane.at<uchar>(y,x)&uchar(bit))?uchar(255):uchar(0);
        }
    }
}

//no worries, no cries
// string GetExtention(string s){
//     reverse(s.begin(), s.end());
//     size_t pos = s.find(".");
    
//     s = s.erase(pos,s.length());
//     reverse(s.begin(), s.end());

//     return s;
// }

int main(int argc, char** argv) {
    cv::Mat INPUT = cv::imread(argv[1], CV_8U);

    // string EXT = GetExtention(argv[1]); 
    int col, row; 
    int x, y;
    col = INPUT.cols;
    row = INPUT.rows;  

    // PLANES
    cv::Mat plane0(row, col, CV_8UC1, cv::Scalar(0));
    cv::Mat plane1 = plane0.clone();
    cv::Mat plane2 = plane0.clone();
    cv::Mat plane3 = plane0.clone();
    cv::Mat plane4 = plane0.clone();
    cv::Mat plane5 = plane0.clone();
    cv::Mat plane6 = plane0.clone();
    cv::Mat plane7 = plane0.clone();

    // SLICE
    Slice(INPUT,plane0,1);
    Slice(INPUT,plane1,2);
    Slice(INPUT,plane2,4);
    Slice(INPUT,plane3,8);
    Slice(INPUT,plane4,16);
    Slice(INPUT,plane5,32);
    Slice(INPUT,plane6,64);
    Slice(INPUT,plane7,128);

    // OUTPUT
    cv::imwrite(("./pic/bit_plane0.png"),plane0);
    cv::imwrite(("./pic/bit_plane1.png"),plane1);
    cv::imwrite(("./pic/bit_plane2.png"),plane2);
    cv::imwrite(("./pic/bit_plane3.png"),plane3);
    cv::imwrite(("./pic/bit_plane4.png"),plane4);
    cv::imwrite(("./pic/bit_plane5.png"),plane5);
    cv::imwrite(("./pic/bit_plane6.png"),plane6);
    cv::imwrite(("./pic/bit_plane7.png"),plane7);

    return 1;
}