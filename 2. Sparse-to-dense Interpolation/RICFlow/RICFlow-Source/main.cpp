#define _CRT_SECURE_NO_WARNINGS
#include <chrono>
#include <string>
#include "RIC.h"
#include "./include/Image.h"
#include "OpticFlowIO.h"
#include "opencv2/ximgproc.hpp" // for struct edge detection

//#define DEBUG
#ifdef  DEBUG
#pragma comment(lib, "D:/opencv_contrib/opencv_contrib/x64/vc17/lib/opencv_world460d.lib")
#endif
#ifndef DEBUG
#pragma comment(lib, "D:/opencv_contrib/opencv_contrib/x64/vc17/lib/opencv_world460.lib")
#endif //  DEBUG

/* read semi-dense matches, stored as x1 y1 x2 y2 per line (other values on the same line is not taking into account) */
void ReadMatches(const char* filename, FImage& outMat)
{
	float* tmp = new float[4 * 100000]; // max number of match pair
	FILE* fid;
	fopen_s(&fid, filename, "r");
	int nmatch = 0;
	float x1, x2, y1, y2;
	while (!feof(fid) && fscanf(fid, "%f %f %f %f%*[^\n]", &x1, &y1, &x2, &y2) == 4) {
		tmp[4 * nmatch] = x1;
		tmp[4 * nmatch + 1] = y1;
		tmp[4 * nmatch + 2] = x2;
		tmp[4 * nmatch + 3] = y2;
		nmatch++;
	}
	outMat.allocate(4, nmatch);
	memcpy(outMat.pData, tmp, nmatch * 4 * sizeof(float));
	fclose(fid);
	delete[] tmp;
}

// prepare cost map from Structured Edge Detector
void GetCostMap(const char* imgName, FImage& outCostMap)
{
	cv::Mat cvImg1 = cv::imread(imgName);
	int w = cvImg1.cols;
	int h = cvImg1.rows;
	outCostMap.allocate(w, h, 1);

	cv::Mat fImg1;
	// convert source image to [0-1] range
	cvImg1.convertTo(fImg1, cv::DataType<float>::type, 1 / 255.0);
	int borderSize = 10;
	cv::copyMakeBorder(fImg1, fImg1, borderSize, borderSize, borderSize, borderSize, cv::BORDER_REPLICATE);
	cv::Mat edges(fImg1.size(), fImg1.type());

	std::string filename = "D:/Code-VS/Project-PatchMatch/RICFlow/RICFlow-Source/win32/model.yml.gz";
	cv::Ptr<cv::ximgproc::StructuredEdgeDetection> sEdge = cv::ximgproc::createStructuredEdgeDetection(filename);
	sEdge->detectEdges(fImg1, edges);
	// save result to FImage
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			outCostMap[i * w + j] = edges.at<float>(i + borderSize, j + borderSize);
		}
	}
}

void ReadEdges(char* fileName, FImage& edge)
{
	int w = edge.width();
	int h = edge.height();
	FILE* fp = fopen(fileName, "rb");
	int size = fread(edge.pData, sizeof(float), w * h, fp);
	assert(size == w * h);
	fclose(fp);
}

int main(int argc, char* argv[])
{
	//if (argc < 5) {
	//	printf("USAGE: RIC image1 image2 inMatchText outoptflow\n");
	//	return -1;
	//}

	//std::string img1_path = argv[1];
	//std::string img2_path = argv[2];
	//std::string matches_path = argv[3];
	//std::string optflow_path = argv[4];

	//FImage img1, img2;
	//FImage matches, costMap;

	//img1.imread(img1_path.c_str());
	//GetCostMap(img1_path.c_str(), costMap);
	//img2.imread(img2_path.c_str());

	//// 	costMap.allocate(img1.width(), img1.height(), 1);
	//// 	ReadEdges(argv[3], costMap);

	//ReadMatches(matches_path.c_str(), matches);

	//int w = img1.width();
	//int h = img1.height();
	//if (img2.width() != w || img2.height() != h) {
	//	printf("RIC can only handle images with the same dimension!\n");
	//	return -1;
	//}

	//RIC ric;
	//FImage u, v;
	//ric.SetSuperpixelSize(100);
	//ric.Interpolate(img1, img2, costMap, matches, u, v);

	//// save the flow and the visualization image
	//OpticFlowIO::WriteFlowFile(u.pData, v.pData, w, h, optflow_path.c_str());

	//OpticFlowIO::SaveFlowAsImage(optflow_dis_path.c_str(), u.pData, v.pData, w, h);

	//std::string log_path = "time-ricflow.txt";
	//std::string img1_path = "D:/Code-VS/picture/figs/dataset7-test1.png";
	//std::string img2_path = "D:/Code-VS/picture/figs/dataset7-test1.png";
	//std::string match_path = "D:/Code-VS/picture/figs/dataset7-test1_matches.txt";
	//std::string flow_path = "D:/Code-VS/picture/figs/dataset7-RICFlow.flo";

	//FImage img1, img2;
	//FImage matches, costMap;

	//std::chrono::steady_clock::time_point in_time = std::chrono::steady_clock::now();

	//img1.imread(img1_path.c_str());
	//GetCostMap(img1_path.c_str(), costMap);
	//img2.imread(img2_path.c_str());

	//// 	costMap.allocate(img1.width(), img1.height(), 1);
	//// 	ReadEdges(argv[3], costMap);

	//ReadMatches(match_path.c_str(), matches);

	//int w = img1.width();
	//int h = img1.height();
	//if (img2.width() != w || img2.height() != h) {
	//	printf("RIC can only handle images with the same dimension!\n");
	//	return -1;
	//}

	//RIC ric;
	//FImage u, v;
	//ric.SetSuperpixelSize(100);
	//ric.Interpolate(img1, img2, costMap, matches, u, v);

	//std::chrono::steady_clock::time_point out_time = std::chrono::steady_clock::now();
	//double time_cur = std::chrono::duration<double>(out_time - in_time).count();

	//std::ofstream outfile;
	//outfile.open(log_path, std::ios::out | std::ios::app);
	//outfile << time_cur << std::endl;
	//outfile.close();

	//// save the flow and the visualization image
	//OpticFlowIO::WriteFlowFile(u.pData, v.pData, w, h, flow_path.c_str());
	////OpticFlowIO::SaveFlowAsImage(outName, u.pData, v.pData, w, h);

	if (argc >= 5)
	{
		std::string log_path = "time-ricflow.txt";
		std::string img1_path = argv[1];
		std::string img2_path = argv[2];
		std::string matches_path = argv[3];
		std::string optflow_path = argv[4];

		FImage img1, img2;
		FImage matches, costMap;

		std::chrono::steady_clock::time_point in_time = std::chrono::steady_clock::now();

		img1.imread(img1_path.c_str());
		GetCostMap(img1_path.c_str(), costMap);
		img2.imread(img2_path.c_str());

		// 	costMap.allocate(img1.width(), img1.height(), 1);
		// 	ReadEdges(argv[3], costMap);

		ReadMatches(matches_path.c_str(), matches);

		int w = img1.width();
		int h = img1.height();
		if (img2.width() != w || img2.height() != h) {
			printf("RIC can only handle images with the same dimension!\n");
			return -1;
		}

		RIC ric;
		FImage u, v;
		ric.SetSuperpixelSize(100);
		ric.Interpolate(img1, img2, costMap, matches, u, v);

		std::chrono::steady_clock::time_point out_time = std::chrono::steady_clock::now();
		double time_cur = std::chrono::duration<double>(out_time - in_time).count();

		std::ofstream outfile;
		outfile.open(log_path, std::ios::out | std::ios::app);
		outfile << time_cur << std::endl;
		outfile.close();
		//
			// save the flow and the visualization image
		OpticFlowIO::WriteFlowFile(u.pData, v.pData, w, h, optflow_path.c_str());
		//OpticFlowIO::SaveFlowAsImage(outName, u.pData, v.pData, w, h);
	}
	else
	{
		printf("USAGE: RIC image1.png image2.png matches.txt optflow.flo\n");
		return -1;
	}

	return 0;
}
