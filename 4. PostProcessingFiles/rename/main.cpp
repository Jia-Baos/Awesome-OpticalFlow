#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utils/logger.hpp>

int main(int argc, char *argv[])
{
    std::cout << "Version: " << CV_VERSION << std::endl;
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);

    const std::string target_path = "D:/DataSet/dataset2/data-ppm/";
    const std::filesystem::path root_path = "D:/DataSet/dataset2/data/";
    for (auto const &iter : std::filesystem::directory_iterator(root_path))
    {

        std::string pre_path = iter.path().string();
        std::string cur_path = target_path + iter.path().stem().string() + ".ppm";
        std::cout << "pre_path: " << pre_path << std::endl;
        std::cout << "cur_path: " << cur_path << std::endl;

        cv::Mat img = cv::imread(pre_path);
        cv::imwrite(cur_path, img);
    }

    return 0;
}