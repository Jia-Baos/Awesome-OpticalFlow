#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utils/logger.hpp>

void ImagePadded(const cv::Mat &src,
                 cv::Mat &dst,
                 const int patch_width)
{
    const int w = src.cols;
    const int h = src.rows;

    // Pad the image avoid the block's size euqals 1
    const int width_max_even = patch_width *
                               (static_cast<int>(w / patch_width) + 1);

    const int height_max_even = patch_width *
                                (static_cast<int>(h / patch_width) + 1);

    cv::copyMakeBorder(src, dst, 0,
                       height_max_even - h,
                       0,
                       width_max_even - w,
                       cv::BORDER_REPLICATE);
}

int main(int argc, char *argv[])
{
    std::cout << "Version: " << CV_VERSION << std::endl;
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);

    const std::string target_path = "D:/DataSet/dataset3/temp/";
    const std::filesystem::path root_path = "D:/DataSet/dataset3/temp/";
    for (auto const &iter : std::filesystem::directory_iterator(root_path))
    {
        std::string pre_path = iter.path().string();
        std::string cur_path = target_path + iter.path().stem().string() + ".ppm";
        std::cout << "pre_path: " << pre_path << std::endl;
        std::cout << "cur_path: " << cur_path << std::endl;

        cv::Mat img = cv::imread(pre_path);

        cv::Mat dst;
        // ImagePadded(img, dst, 4);
        cv::resize(img, dst, cv::Size(928, 1136));

        cv::imwrite(cur_path, dst);
    }

    return 0;
}