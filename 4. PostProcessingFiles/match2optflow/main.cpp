#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <opencv2/opencv.hpp>

// value to use to represent unknown flow
#define UNKNOWN_FLOW 1e10

// first four bytes, should be the same in little endian
#define TAG_FLOAT 202021.25 // check for this when READING the file
#define TAG_STRING "PIEH"   // use this when WRITING the file

// write a 2-band image into flow file
void WriteFlowFile(cv::Mat &img, const char *filename)
{
    if (filename == NULL)
        throw "WriteFlowFile: empty filename";

    const char *dot = strrchr(filename, '.');
    if (dot == NULL)
        throw "WriteFlowFile: extension required in filename";

    if (strcmp(dot, ".flo") != 0)
        throw "WriteFlowFile: filename '%s' should have extension";

    int width = img.cols;
    int height = img.rows;
    int nBands = img.channels();

    if (nBands != 2)
        throw "WriteFlowFile: image must have 2 bands";

    FILE *stream;
    fopen_s(&stream, filename, "wb");
    if (stream == 0)
        throw "WriteFlowFile: could not open";

    // write the header
    fprintf(stream, TAG_STRING);
    if ((int)fwrite(&width, sizeof(int), 1, stream) != 1 ||
        (int)fwrite(&height, sizeof(int), 1, stream) != 1)
        throw "WriteFlowFile: problem writing header";

    // write the data
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            float *img_pointer = img.ptr<float>(i, j);
            fwrite(&img_pointer[0], sizeof(float), 1, stream);
            fwrite(&img_pointer[1], sizeof(float), 1, stream);
        }
    }

    fclose(stream);
}

int main(int argc, char *argv[])
{
    std::cout << "Version: " << CV_VERSION << std::endl;

    std::string templ_path = "D:/Code-VS/picture/figs/template.png";

    cv::Mat templ = cv::imread(templ_path);
    std::cout << "templ size: " << templ.size() << std::endl;

    const int radius = 7;
    const int w = templ.cols;
    const int h = templ.rows;

    const std::filesystem::path root_path = "D:/Code-VS/picture/figs/";
    for (auto const &iter : std::filesystem::directory_iterator(root_path))
    {

        if (iter.path().extension().string() == ".txt")
        {
            cv::Mat optflow = cv::Mat(templ.size(), CV_32FC2, cv::Scalar(UNKNOWN_FLOW, UNKNOWN_FLOW));

            const std::string match_path = iter.path().string();
            const std::string optflow_path = root_path.string() + iter.path().stem().string() + ".flo";
            std::cout << "match_path: " << match_path << std::endl;
            std::cout << "optflow_path: " << optflow_path << std::endl;

            std::ifstream infile;
            infile.open(match_path, std::ios::in);

            // 获取每一行的内容
            int x1 = 0;
            int y1 = 0;
            int x2 = 0;
            int y2 = 0;
            std::string line_content;
            while (std::getline(infile, line_content))
            {
                // 将每一行的坐标按空格进行分割
                sscanf(line_content.c_str(), "%d %d %d %d %*[^/n]", &x1, &y1, &x2, &y2);

                for (int dy = -radius; dy <= radius; dy++)
                {
                    for (int dx = -radius; dx <= radius; dx++)
                    {

                        const int x = std::max(0, std::min(static_cast<int>(x1 + dx + 0.5f), w - 1));
                        const int y = std::max(0, std::min(static_cast<int>(y1 + dy + 0.5f), h - 1));
                        const float flow_x = static_cast<float>(x2 - x1);
                        const float flow_y = static_cast<float>(y2 - y1);

                        optflow.at<cv::Vec2f>(y, x)[0] = flow_x;
                        optflow.at<cv::Vec2f>(y, x)[1] = flow_y;
                    }
                }
            }

            infile.close();

            WriteFlowFile(optflow, optflow_path.c_str());
        }
    }

    return 0;
}