#ifndef __EVALFUNC_H__
#define  __EVALFUNC_H__

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <exception>

#include "./include/opencv2/opencv.hpp"

// the "official" threshold - if the absolute value of either 
// flow component is greater, it's considered unknown
#define UNKNOWN_FLOW_THRESH 1e9

// value to use to represent unknown flow
#define UNKNOWN_FLOW 1e10

// return whether flow vector is unknown
bool unknown_flow(float u, float v);
bool unknown_flow(float* f);

// read a flow file into 2-band image
void ReadFlowFile(cv::Mat& img, const char* filename);

cv::Mat endpointError(const cv::Mat &flow1, const cv::Mat &flow2);

float AEE(const cv::Mat &errors);

int stat_RX(const cv::Mat &errors, float threshold);

#endif //!__EVALFUNC_H__