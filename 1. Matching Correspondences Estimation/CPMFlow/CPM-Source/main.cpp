#define _CRT_SECURE_NO_WARNINGS
#include "CPM.h"
#include "OpticFlowIO.h"

#include <string>
#include <chrono>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

#ifdef  _DEBUG
#pragma comment(lib, "D:/opencv-4.6.0-contrib/build/x64/vc17/lib/opencv_world460d.lib")
#else
#pragma comment(lib, "D:/opencv-4.6.0-contrib/build/x64/vc17/lib/opencv_world460.lib")
#endif // !DEBUG

// draw each match as a 3x3 color block
void Match2Flow(FImage& inMat, FImage& ou, FImage& ov, int w, int h)
{
	if (!ou.matchDimension(w, h, 1)) {
		ou.allocate(w, h, 1);
	}
	if (!ov.matchDimension(w, h, 1)) {
		ov.allocate(w, h, 1);
	}
	ou.setValue(UNKNOWN_FLOW);
	ov.setValue(UNKNOWN_FLOW);
	int cnt = inMat.height();
	for (int i = 0; i < cnt; i++) {
		float* p = inMat.rowPtr(i);
		float x = p[0];
		float y = p[1];
		float u = p[2] - p[0];
		float v = p[3] - p[1];
		for (int di = -1; di <= 1; di++) {
			for (int dj = -1; dj <= 1; dj++) {
				int tx = ImageProcessing::EnforceRange(x + dj, w);
				int ty = ImageProcessing::EnforceRange(y + di, h);
				ou[ty * w + tx] = u;
				ov[ty * w + tx] = v;
			}
		}
	}
}

void WriteMatches(const char* filename, FImage& inMat)
{
	int len = inMat.height();
	FILE* fid = fopen(filename, "w");
	for (int i = 0; i < len; i++) {
		float x1 = inMat[4 * i + 0];
		float y1 = inMat[4 * i + 1];
		float x2 = inMat[4 * i + 2];
		float y2 = inMat[4 * i + 3];
		fprintf(fid, "%.0f %.0f %.0f %.0f\n", x1, y1, x2, y2);
		//fprintf(fid, "%.3f %.3f %.3f %.3f 1 100\n", x1, y1, x2, y2);
	}
	fclose(fid);
}

int main(int argc, char* argv[])
{
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);

	//const std::string matches_path = "E:/paper1-dataset/matches-cpm/";
	//const std::string templ_path = "E:/paper1-dataset/template.png";

	//int img_num = 0;
	//double time_avg = 0.0;

	//const std::filesystem::path root_path = "E:/paper1-dataset/test/";
	//for (auto const& iter : std::filesystem::directory_iterator(root_path))
	//{
	//	const std::string src_path = iter.path().string();
	//	std::cout << src_path << std::endl;

	//	const std::string match_path = matches_path + iter.path().stem().string() + ".txt";

	//	std::chrono::steady_clock::time_point in_time = std::chrono::steady_clock::now();

	//	FImage img1, img2;

	//	img1.imread(templ_path.c_str());
	//	img2.imread(src_path.c_str());

	//	int step = 3;
	//	if (argc >= 5) {
	//		step = atoi(argv[4]);
	//	}

	//	int w = img1.width();
	//	int h = img1.height();
	//	if (img2.width() != w || img2.height() != h) {
	//		printf("CPM can only handle images with the same dimension!\n");
	//		return -1;
	//	}

	//	CTimer totalT;
	//	FImage matches;

	//	CPM cpm;
	//	cpm.SetStep(step);
	//	cpm.Matching(img1, img2, matches);
	//	totalT.toc("total time: ");

	//	std::chrono::steady_clock::time_point out_time = std::chrono::steady_clock::now();
	//	double time_cur = std::chrono::duration<double>(out_time - in_time).count();
	//	std::cout << "Spend time: " << time_cur << std::endl;

	//	// –¥»Îmatches.txt
	//	WriteMatches(match_path.c_str(), matches);

	//	time_avg = time_avg * img_num + time_cur;
	//	img_num += 1;
	//	time_avg /= img_num;
	//}

	//std::cout << "time avg: " << time_avg << std::endl;

	//std::string img1_path = "E:\\paper4-dataset\\template.ppm";
	//std::string img2_path = "E:\\paper4-dataset\\test-ppm\\204.ppm";

	//std::string temp_path = "E:\\paper4-dataset\\matches-cpm\\";
	//const std::string matches_path = temp_path + "204-matches-cpm.txt";
	//const std::string optflow_path = temp_path + "-optflow-cpm.flo";
	//const std::string optflow_dis_path = temp_path + "-optflow-cpm-dis.png";

	std::chrono::steady_clock::time_point in_time = std::chrono::steady_clock::now();
	std::string img1_path =
		"D:\\Code-VS\\picture\\Demons\\lena3.ppm";
	std::string img2_path =
		"D:\\Code-VS\\picture\\Demons\\lena4.ppm";
	std::string temp_path = "D:\\Code-VS\\picture\\Demons\\";
	const std::string matches_path = temp_path + "matches-cpm.txt";

	/*std::string img1_path =
		"D:\\CmakeProject\\OpticalFlow\\DeepMatching\\climb\\climb1.ppm";
	std::string img2_path =
		"D:\\CmakeProject\\OpticalFlow\\DeepMatching\\climb\\climb2.ppm";
	std::string temp_path = "D:\\CmakeProject\\OpticalFlow\\DeepMatching\\climb\\";
	const std::string matches_path = temp_path + "matches-cpm.txt";*/

	FImage img1, img2;

	img1.imread(img1_path.c_str());
	img2.imread(img2_path.c_str());

	int step = 3;
	if (argc >= 5) {
		step = atoi(argv[4]);
	}

	int w = img1.width();
	int h = img1.height();
	if (img2.width() != w || img2.height() != h) {
		printf("CPM can only handle images with the same dimension!\n");
		return -1;
	}

	//CTimer totalT;
	FImage matches;

	CPM cpm;
	cpm.SetStep(step);
	cpm.Matching(img1, img2, matches);

	//totalT.toc("total time: ");

	// –¥»Îmatches.txt
	WriteMatches(matches_path.c_str(), matches);
	std::chrono::steady_clock::time_point out_time = std::chrono::steady_clock::now();
	double spend_time = std::chrono::duration<double>(out_time - in_time).count();
	std::cout << "spend_time: " << spend_time << std::endl;

	/*FImage u, v;
	Match2Flow(matches, u, v, w, h);
	OpticFlowIO::SaveFlowAsImage(optflow_dis_path.c_str(), u.pData, v.pData, w, h);
	OpticFlowIO::WriteFlowFile(u.pData, v.pData, w, h, optflow_path.c_str());*/

	//if (argc < 4) {
	//	printf("USAGE: CPM image1 image2 outMatchText <step>\n");
	//	return -1;
	//}

	//std::string img1_path = argv[1];
	//std::string img2_path = argv[2];
	//std::string matches_path = argv[3];

	//FImage img1, img2;

	//img1.imread(img1_path.c_str());
	//img2.imread(img2_path.c_str());
	//int step = 3;
	//if (argc >= 5) {
	//	step = atoi(argv[4]);
	//}

	//int w = img1.width();
	//int h = img1.height();
	//if (img2.width() != w || img2.height() != h) {
	//	printf("CPM can only handle images with the same dimension!\n");
	//	return -1;
	//}

	//CTimer totalT;
	//FImage matches;

	//CPM cpm;
	//cpm.SetStep(step);
	//cpm.Matching(img1, img2, matches);

	//totalT.toc("total time: ");

	//// –¥»Îmatches.txt
	//WriteMatches(matches_path.c_str(), matches);

	//FImage u, v;
	//Match2Flow(matches, u, v, w, h);
	//OpticFlowIO::WriteFlowFile(u.pData, v.pData, w, h, optflow_path.c_str());
	//OpticFlowIO::SaveFlowAsImage(optflow_dis_path.c_str(), u.pData, v.pData, w, h);

	return 0;
}
