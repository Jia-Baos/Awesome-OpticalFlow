#include "./evalFunc.hpp"

// first four bytes, should be the same in little endian
#define TAG_FLOAT 202021.25 // check for this when READING the file
#define TAG_STRING "PIEH"   // use this when WRITING the file

// return whether flow vector is unknown
bool unknown_flow(float u, float v)
{
    return (fabs(u) > UNKNOWN_FLOW_THRESH) || (fabs(v) > UNKNOWN_FLOW_THRESH) || isnan(u) || isnan(v);
}

bool unknown_flow(float *f)
{
    return unknown_flow(f[0], f[1]);
}

// read a flow file into 2-band image
void ReadFlowFile(cv::Mat &img, const char *filename)
{
    if (filename == NULL)
        throw "ReadFlowFile: empty filename";

    const char *dot = strrchr(filename, '.');
    if (strcmp(dot, ".flo") != 0)
        throw "ReadFlowFile: extension .flo expected";

    FILE *stream;
    fopen_s(&stream, filename, "rb");
    if (stream == 0)
        throw "ReadFlowFile: could not open";

    int width, height;
    float tag;

    if ((int)fread(&tag, sizeof(float), 1, stream) != 1 ||
        (int)fread(&width, sizeof(int), 1, stream) != 1 ||
        (int)fread(&height, sizeof(int), 1, stream) != 1)
        throw "ReadFlowFile: problem reading file";

    if (tag != TAG_FLOAT) // simple test for correct endian-ness
        throw "ReadFlowFile: wrong tag (possibly due to big-endian machine?)";

    // another sanity check to see that integers were read correctly (99999 should do the trick...)
    if (width < 1 || width > 99999)
        throw "ReadFlowFile: illegal width";

    if (height < 1 || height > 99999)
        throw "ReadFlowFile: illegal height";

    img = cv::Mat(cv::Size(width, height), CV_32FC2);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            float *img_pointer = img.ptr<float>(i, j);
            fread(&img_pointer[0], sizeof(float), 1, stream);
            fread(&img_pointer[1], sizeof(float), 1, stream);
        }
    }

    if (fgetc(stream) != EOF)
        throw "ReadFlowFile: file is too long";

    fclose(stream);
}

inline bool isFlowCorrect(const cv::Vec2f &u)
{
    return !cvIsNaN(u[0]) && !cvIsNaN(u[1]) && (fabs(u[0]) < 1e9) && (fabs(u[1]) < 1e9);
}

cv::Mat endpointError(const cv::Mat &flow1, const cv::Mat &flow2)
{
    const int w = flow1.cols;
    const int h = flow1.rows;
    cv::Mat result = cv::Mat::zeros(flow1.size(), CV_32FC1);
    for (int i = 0; i < h; ++i)
    {
        for (int j = 0; j < w; ++j)
        {
            const cv::Vec2f u1 = flow1.at<cv::Vec2f>(i, j);
            const cv::Vec2f u2 = flow2.at<cv::Vec2f>(i, j);

            if (isFlowCorrect(u1) && isFlowCorrect(u2))
            {
                const cv::Vec2f diff = u1 - u2;
                result.at<float>(i, j) = sqrt((float)diff.ddot(diff)); // distance
            }
            else
                result.at<float>(i, j) = std::numeric_limits<float>::quiet_NaN();
        }
    }
    return result;
}

float AEE(const cv::Mat &errors)
{
    int count = 0;
    float aee = 0.0;
    const int w = errors.cols;
    const int h = errors.rows;
    for (int i = 0; i < h; ++i)
    {
        for (int j = 0; j < w; ++j)
        {
            aee += errors.at<float>(i, j);
            ++count;
        }
    }
    return aee / count;
}

// what fraction of pixels have errors higher than given threshold
int stat_RX(const cv::Mat &errors, float threshold)
{
    int count = 0;
    const int w = errors.cols;
    const int h = errors.rows;
    for (int i = 0; i < h; ++i)
    {
        for (int j = 0; j < w; ++j)
        {
            if (errors.at<float>(i, j) <= threshold)
                ++count;
        }
    }
    return count;
}
