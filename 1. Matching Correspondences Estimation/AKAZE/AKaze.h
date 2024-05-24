#pragma once
#ifndef __AKAZE_H__
#define __AKAZE_H__

#include <fstream>
#include <opencv2/opencv.hpp>

void AKazeRegistration(const cv::Mat image_object,
	const cv::Mat image_scene,
	std::vector<cv::Point2f>& object_keypoints_inliers,
	std::vector<cv::Point2f>& scene_keypoints_inliers);

void WriteMatchesFile(const std::vector<cv::Point2f>& seeds1,
	const std::vector<cv::Point2f>& seeds2,
	const std::string matches_path);

#endif // !__AKAZE_H__
