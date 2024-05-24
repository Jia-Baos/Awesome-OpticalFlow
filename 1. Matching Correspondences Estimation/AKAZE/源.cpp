#include <iostream>
#include <string>
#include <chrono>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

#include "AKaze.h"

#ifdef _DEBUG
#pragma comment(lib, "D:/opencv_contrib/opencv_contrib/x64/vc17/lib/opencv_world460d.lib")
#else
#pragma comment(lib, "D:/opencv_contrib/opencv_contrib/x64/vc17/lib/opencv_world460.lib")
#endif // _DEBUG

int main(int argc, char* argv[])
{
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);

	const std::string matches_path = "D:/DataSet/dataset1/matches-akaze/";
	const std::string templ_path = "D:/DataSet/dataset1/template/template2-resized.png";
	cv::Mat templ = cv::imread(templ_path);
	cv::Mat templ_gray = cv::Mat::zeros(templ.size(), CV_8UC1);
	cv::cvtColor(templ, templ_gray, cv::COLOR_BGR2GRAY);

	int img_num = 0;
	double time_avg = 0.0;

	const std::filesystem::path root_path = "D:/DataSet/dataset1/data/";
	for (auto const& iter : std::filesystem::directory_iterator(root_path))
	{
		const std::string src_path = iter.path().string();
		std::cout << src_path << std::endl;

		std::chrono::steady_clock::time_point in_time = std::chrono::steady_clock::now();

		cv::Mat tested_image = cv::imread(src_path);
		cv::cvtColor(tested_image, tested_image, cv::COLOR_BGR2GRAY);

		std::vector<cv::Point2f> object_keypoints_inliers;
		std::vector<cv::Point2f> scene_keypoints_inliers;
		AKazeRegistration(templ_gray, tested_image, object_keypoints_inliers, scene_keypoints_inliers);

		std::chrono::steady_clock::time_point out_time = std::chrono::steady_clock::now();
		double time_cur = std::chrono::duration<double>(out_time - in_time).count();

		std::cout << "Spend time: " << time_cur << std::endl;
		std::cout << "matches's size: " << object_keypoints_inliers.size() << std::endl;

		const std::string match_path = matches_path + iter.path().stem().string() + ".txt";
		WriteMatchesFile(object_keypoints_inliers, scene_keypoints_inliers, match_path);

		time_avg = time_avg * img_num + time_cur;
		img_num += 1;
		time_avg /= img_num;
	}

	std::cout << "time avg: " << time_avg << std::endl;

	return 0;
}
