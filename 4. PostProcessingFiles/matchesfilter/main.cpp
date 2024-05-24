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

    const std::string target_path = "D:/DataSet/dataset5/data-optflow-acpm7-dm/";
    const std::string templ_img_path = "D:/DataSet/dataset5/template/template1-resized.png";

    cv::Mat img = cv::imread(templ_img_path);
    const int w = img.cols;
    const int h = img.rows;

    const std::filesystem::path root_path = "D:/DataSet/dataset5/data-matches-acpm7/";
    for (auto const &iter : std::filesystem::directory_iterator(root_path))
    {

        const std::string matches_path = iter.path().string();
        const std::string matches_filter_path = target_path + iter.path().stem().string() + ".txt";

        std::cout << "matches_path: " << matches_path << std::endl;
        std::cout << "matches_filter_path: " << matches_filter_path << std::endl;

        std::ifstream infile;
        infile.open(matches_path, std::ios::in);

        // 过滤后的matches
        std::ofstream outfile;
        outfile.open(matches_filter_path, std::ios::out | std::ios::app);

        int num1 = 0;
        int num2 = 0;
        // 获取每一行的内容
        int x1 = 0;
        int y1 = 0;
        int x2 = 0;
        int y2 = 0;
        std::string line_content;
        while (std::getline(infile, line_content))
        {
            num1++;
            // 将每一行的坐标按空格进行分割
            sscanf(line_content.c_str(), "%d %d %d %d/n", &x1, &y1, &x2, &y2);

            if (x1 < 0 || x1 > w - 1 || y1 < 0 || y1 > h - 1 || x2 < 0 || x2 > w - 1 || y2 < 0 || y2 > h - 1)
            {
                continue;
            }
            else
            {
                num2++;
                outfile << x1 << " "
                        << y1 << " "
                        << x2 << " "
                        << y2 << std::endl;
            }
        }
        outfile.close();
        infile.close();

        std::cout << "num1: " << num1 << std::endl;
        std::cout << "num2: " << num2 << std::endl;
    }

    return 0;
}