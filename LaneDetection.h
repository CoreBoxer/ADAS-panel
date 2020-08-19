#ifndef LANEDETECTION_H
#define LANEDETECTION_H

#include <iostream>
#include <numeric>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class LaneDetection
{
public:
    LaneDetection();
    ~LaneDetection();

    Mat image_src;
    Mat image_dst;

    Mat process(Mat image);
};

#endif // LANEDETECTION_H
