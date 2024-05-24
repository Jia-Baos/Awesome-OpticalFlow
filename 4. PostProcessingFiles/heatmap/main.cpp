#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

#ifdef _DEBUG
#pragma comment(lib, "D:/opencv_contrib/opencv_contrib/x64/vc17/lib/opencv_world460d.lib")
#else
#pragma comment(lib, "D:/opencv_contrib/opencv_contrib/x64/vc17/lib/opencv_world460.lib")
#endif // _DEBUG

void ImagePadded(const cv::Mat& src1,
	const cv::Mat& src2,
	cv::Mat& dst1,
	cv::Mat& dst2,
	const int patch_width,
	const int patch_height)
{
	const int width_max = src1.cols > src2.cols ? src1.cols : src2.cols;
	const int height_max = src1.rows > src2.rows ? src1.rows : src2.rows;

	// Pad the image avoid the block's size euqals 1
	const int width_max_even = patch_width *
		(static_cast<int>(width_max / patch_width) + 1);
	const int height_max_even = patch_height *
		(static_cast<int>(height_max / patch_height) + 1);

	cv::resize(src1, dst1, cv::Size(width_max_even, height_max_even));
	cv::resize(src2, dst2, cv::Size(width_max_even, height_max_even));
}

int main(int argc, char* argv[])
{
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);

	const std::string fixed_image_path = "D:/Code-VS/picture/figs/dataset6-test1.jpg";
	const std::string moved_image_path = "D:/Code-VS/picture/figs/dataset6-test2.jpg";
	const std::string fixed_image_padded_path = "D:/Code-VS/picture/figs/dataset6-test1.png";
	const std::string moved_image_padded_path = "D:/Code-VS/picture/figs/dataset6-test2.png";
	const std::string roi1_path = "D:/Code-VS/picture/figs/roi1.png";
	const std::string roi2_path = "D:/Code-VS/picture/figs/roi2.png";
	const std::string heatmap_path = "D:/Code-VS/picture/figs/heatmap.png";

	cv::Mat fixed_image = cv::imread(fixed_image_path);
	cv::Mat moved_image = cv::imread(moved_image_path);
	cv::cvtColor(fixed_image, fixed_image, cv::COLOR_BGR2GRAY);
	cv::cvtColor(moved_image, moved_image, cv::COLOR_BGR2GRAY);

	cv::Mat fixed_image_padded, moved_image_padded;
	ImagePadded(fixed_image, moved_image, fixed_image_padded, moved_image_padded, 4, 4);

	const int x = 160;
	const int y = 178;
	const int width1 = 16;
	const int width2 = 60;
	cv::Mat roi1 = fixed_image_padded(cv::Rect(x - width1, x - width1, 2 * width1, 2 * width1));
	cv::Mat roi2 = moved_image_padded(cv::Rect(x - width2, x - width2, 2 * width2, 2 * width2));

	cv::Mat fixed_image_padded_temp = fixed_image_padded.clone();
	cv::rectangle(fixed_image_padded_temp, cv::Point2f(x - 8, x - 8), cv::Point2f(x + 8, x + 8), cv::Scalar(255), 1, 8);
	cv::rectangle(fixed_image_padded_temp, cv::Point2f(x - 12, x - 12), cv::Point2f(x + 12, x + 12), cv::Scalar(255), 1, 8);
	cv::rectangle(fixed_image_padded_temp, cv::Point2f(x - 16, x - 16), cv::Point2f(x + 16, x + 16), cv::Scalar(255), 1, 8);
	cv::Mat roi1_temp = fixed_image_padded_temp(cv::Rect(x - width2, x - width2, 2 * width2, 2 * width2));

	// Template matching
	cv::Size result_size = cv::Size(roi2.cols - roi1.cols + 1,
		roi2.rows - roi1.rows + 1);
	cv::Mat result = cv::Mat::zeros(result_size, CV_32FC1);
	cv::matchTemplate(roi2, roi1, result, cv::TM_CCOEFF_NORMED);
	cv::normalize(result, result, 0.0, 1.0, cv::NORM_MINMAX);

	cv::Mat heatmap;
	result.convertTo(heatmap, CV_8UC1, 255.0);
	cv::applyColorMap(heatmap, heatmap, cv::COLORMAP_JET);

	cv::imwrite(fixed_image_padded_path, fixed_image_padded);
	cv::imwrite(moved_image_padded_path, moved_image_padded);
	cv::imwrite(roi1_path, roi1_temp);
	cv::imwrite(roi2_path, roi2);
	cv::imwrite(heatmap_path, heatmap);
	cv::namedWindow("heatmap", cv::WINDOW_GUI_NORMAL);
	cv::imshow("heatmap", heatmap);

	cv::waitKey();
	return 0;
}
