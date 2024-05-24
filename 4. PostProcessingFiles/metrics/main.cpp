#include <filesystem>
#include <fstream>
#include <iostream>
#include <opencv2/core/utils/logger.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <string>

// first four bytes, should be the same in little endian
#define TAG_FLOAT 202021.25 // check for this when READING the file

// read a flow file into 2-band image
void ReadFlowFile(cv::Mat &img, const char *filename) {
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

  // another sanity check to see that integers were read correctly (99999 should
  // do the trick...)
  if (width < 1 || width > 99999)
    throw "ReadFlowFile: illegal width";

  if (height < 1 || height > 99999)
    throw "ReadFlowFile: illegal height";

  img = cv::Mat(cv::Size(width, height), CV_32FC2);

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      float *img_pointer = img.ptr<float>(i, j);
      fread(&img_pointer[0], sizeof(float), 1, stream);
      fread(&img_pointer[1], sizeof(float), 1, stream);
    }
  }

  if (fgetc(stream) != EOF)
    throw "ReadFlowFile: file is too long";

  fclose(stream);
}

// 配准metrics
float MSE(const cv::Mat &src1, const cv::Mat &src2) {
  cv::Mat src1_copy = src1.clone();
  cv::Mat src2_copy = src2.clone();

  src1_copy.convertTo(src1_copy, CV_32FC1);
  src2_copy.convertTo(src2_copy, CV_32FC1);

  // Compute the mse_numerater
  cv::Mat mse_numerater_matrix;
  cv::absdiff(src1_copy, src2_copy, mse_numerater_matrix);
  mse_numerater_matrix = mse_numerater_matrix.mul(mse_numerater_matrix);
  float mse_numerater = cv::sum(mse_numerater_matrix)[0];

  // Compute the MSE
  float mse_denominator = src1_copy.cols * src1_copy.rows;
  float my_mse = mse_numerater / mse_denominator;

  return my_mse;
}

float AverageValue(const cv::Mat &src) {
  cv::Mat src_copy = src.clone();
  float src_copy_sum = cv::sum(src_copy)[0];
  float src_copy_average = src_copy_sum / (src_copy.cols * src_copy.rows);

  return src_copy_average;
}

float CorrelationCoefficient(const cv::Mat &src1, const cv::Mat &src2) {
  cv::Mat src1_copy = src1.clone();
  cv::Mat src2_copy = src2.clone();

  src1_copy.convertTo(src1_copy, CV_32FC1);
  src2_copy.convertTo(src2_copy, CV_32FC1);

  float src_1_copy_average = AverageValue(src1_copy);
  float src_2_copy_average = AverageValue(src2_copy);

  cv::Mat src_1_copy_subtraction = src1_copy - src_1_copy_average;
  cv::Mat src_2_copy_subtraction = src2_copy - src_2_copy_average;

  cv::Mat correlation_coefficient_numerater_matrix =
      src_1_copy_subtraction.mul(src_2_copy_subtraction);
  float correlation_coefficient_numerater =
      cv::sum(correlation_coefficient_numerater_matrix)[0];

  cv::Mat correlation_coefficient_denominator_matrix1 =
      src_1_copy_subtraction.mul(src_1_copy_subtraction);
  cv::Mat correlation_coefficient_denominator_matrix2 =
      src_2_copy_subtraction.mul(src_2_copy_subtraction);
  float correlation_coefficient_denominator1 =
      cv::sum(correlation_coefficient_denominator_matrix1)[0];
  float correlation_coefficient_denominator2 =
      cv::sum(correlation_coefficient_denominator_matrix2)[0];
  float correlation_coefficient_denominator =
      sqrt(correlation_coefficient_denominator1 *
           correlation_coefficient_denominator2);

  float my_correlation_coefficient =
      correlation_coefficient_numerater / correlation_coefficient_denominator;

  return my_correlation_coefficient;
}

float Entropy(const cv::Mat &src) {
  cv::Mat src_copy = src.clone();
  float gray_array[256] = {0.0};

  // Compute the nums of every gray value
  for (int i = 0; i < src_copy.rows; i++) {
    uchar *src_copy_poniter = src_copy.ptr<uchar>(i);

    for (int j = 0; j < src_copy.cols; j++) {
      int gray_value = src_copy_poniter[j];
      gray_array[gray_value]++;
    }
  }

  // Compute the probability of every gray value and entropy
  float my_entropy = 0;
  float *gray_array_pointer = gray_array;
  for (int i = 0; i < 255; i++) {
    float gray_value_prob =
        *gray_array_pointer / (src_copy.cols * src_copy.rows);

    if (gray_value_prob != 0) {
      my_entropy = my_entropy - gray_value_prob * log(gray_value_prob);
    } else {
      my_entropy = my_entropy;
    }
    gray_array_pointer++;
  }

  return my_entropy;
}

float JointEntropy(const cv::Mat &src1, const cv::Mat &src2) {
  cv::Mat src1_copy = src1.clone();
  cv::Mat src2_copy = src2.clone();
  float gray_array[256][256] = {0.0};

  // Compute the nums of every gray value pair
  for (int i = 0; i < src1_copy.rows; i++) {
    uchar *src1_copy_poniter = src1_copy.ptr<uchar>(i);
    uchar *src2_copy_poniter = src2_copy.ptr<uchar>(i);

    for (int j = 0; j < src1_copy.cols; j++) {
      int gray_value1 = src1_copy_poniter[j];
      int gray_value2 = src2_copy_poniter[j];
      gray_array[gray_value1][gray_value2]++;
    }
  }

  // Compute the joint_entropy
  // ()优先级高，说明gray_array_pointer是一个指针，指向一个double类型的一维数组，其长度是256
  // 256也可以说是gray_array_pointer的步长，也就是说执行 p + 1
  // 时，gray_array_poniter要跨过256个double类型的长度
  float my_joint_entropy = 0;
  float(*gray_array_pointer)[256] = gray_array;

  for (int i = 0; i < 255; i++) {
    for (int j = 0; j < 255; j++) {
      // *(*(gray_array + i) + j)
      float gray_value_prob =
          gray_array_pointer[i][j] / (src1_copy.cols * src1_copy.rows);

      if (gray_value_prob != 0) {
        my_joint_entropy =
            my_joint_entropy - gray_value_prob * log(gray_value_prob);
      } else {
        my_joint_entropy = my_joint_entropy;
      }
    }
  }

  return my_joint_entropy;
}

float MutualInformation(const cv::Mat &src1, const cv::Mat &src2) {
  cv::Mat src1_copy = src1.clone();
  cv::Mat src2_copy = src2.clone();

  float entropy1 = Entropy(src1_copy);
  float entropy2 = Entropy(src2_copy);
  float joint_entropy = JointEntropy(src1_copy, src2_copy);
  float my_mutual_information = entropy1 + entropy2 - joint_entropy;

  return my_mutual_information;
}

float NormalizedMutualInformation(const cv::Mat &src1, const cv::Mat &src2) {
  cv::Mat src1_copy = src1.clone();
  cv::Mat src2_copy = src2.clone();

  float entropy1 = Entropy(src1_copy);
  float entropy2 = Entropy(src2_copy);
  float mutual_information = MutualInformation(src1_copy, src2_copy);
  float my_normalized_mutual_information =
      2 * mutual_information / (entropy1 + entropy2);

  return my_normalized_mutual_information;
}

float MSSIM(const cv::Mat &src1, const cv::Mat &src2) {
  cv::Mat src1_copy = src1.clone();
  cv::Mat src2_copy = src2.clone();

  int win_size = 11;
  float sigma = 1.5;
  int dynamic_range = 255;
  float K1 = 0.01, K2 = 0.03;

  src1_copy.convertTo(src1_copy, CV_32FC1);
  src2_copy.convertTo(src2_copy, CV_32FC1);

  // 计算两幅图的平均图
  // ux，uy 的每个像素代表以它为中心的滑窗下所有像素的均值(加权) E(X), E(Y)
  cv::Mat mu_x, mu_y;
  cv::GaussianBlur(src1_copy, mu_x, cv::Size(win_size, win_size), sigma); // u_x
  cv::GaussianBlur(src2_copy, mu_y, cv::Size(win_size, win_size), sigma); // u_y

  // compute(weighted) variancesand covariances
  // 计算 E(X^2), E(Y^2), E(XY)
  cv::Mat mu_xx, mu_yy, mu_xy;
  cv::GaussianBlur(src1_copy.mul(src1_copy), mu_xx,
                   cv::Size(win_size, win_size), sigma); // u_xx
  cv::GaussianBlur(src2_copy.mul(src2_copy), mu_yy,
                   cv::Size(win_size, win_size), sigma); // u_yy
  cv::GaussianBlur(src1_copy.mul(src2_copy), mu_xy,
                   cv::Size(win_size, win_size), sigma); // u_xy

  // 进行无偏估计
  float conv_norm = win_size * win_size / (win_size * win_size - 1);
  // sigma_xx = E(X^2) - E(X)^2
  cv::Mat sigma_xx = conv_norm * (mu_xx - (mu_x.mul(mu_x)));
  // sigma_yy = E(Y^2) - E(Y)^2
  cv::Mat sigma_yy = conv_norm * (mu_yy - (mu_y.mul(mu_y)));
  // sigma_xy = E(XY) - E(X)E(Y)
  cv::Mat sigma_xy = conv_norm * (mu_xy - (mu_x.mul(mu_y)));

  // paper 中的公式
  float C1 = K1 * K1 * dynamic_range * dynamic_range;
  float C2 = K2 * K2 * dynamic_range * dynamic_range;

  // paper 中的公式
  cv::Mat A1 = 2 * mu_x.mul(mu_y) + C1;
  cv::Mat A2 = 2 * sigma_xy + C2;
  cv::Mat B1 = mu_x.mul(mu_x) + mu_y.mul(mu_y) + C1;
  cv::Mat B2 = sigma_xx + sigma_yy + C2;

  cv::Mat mssim_matrix;
  // 矩阵中对应位置作除法
  cv::divide(A1.mul(A2), B1.mul(B2), mssim_matrix);

  float my_ssim = cv::mean(mssim_matrix)[0];

  return my_ssim;
}

// 光流场重映射
void MovePixels(const cv::Mat &src, cv::Mat &dst, const cv::Mat &flow,
                const int interpolation) {
  std::vector<cv::Mat> flow_spilit;
  cv::split(flow, flow_spilit);

  // 像素重采样实现
  cv::Mat Tx_map(src.size(), CV_32FC1, 0.0);
  cv::Mat Ty_map(src.size(), CV_32FC1, 0.0);

  for (int i = 0; i < src.rows; i++) {
    float *p_Tx_map = Tx_map.ptr<float>(i);
    float *p_Ty_map = Ty_map.ptr<float>(i);
    for (int j = 0; j < src.cols; j++) {
      p_Tx_map[j] = j + flow_spilit[0].ptr<float>(i)[j];
      p_Ty_map[j] = i + flow_spilit[1].ptr<float>(i)[j];
    }
  }

  cv::remap(src, dst, Tx_map, Ty_map, interpolation);
}

int main(int argc, char *argv[]) {
  std::cout << "Version: " << CV_VERSION << std::endl;
  cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);

  const std::string metric_path =
      "F:/DataSet/dataset2/metrics-acpm13-epicflow.txt";
  const std::string res_img_path = "F:/DataSet/dataset2/res-acpm7-epicflow/";
  const std::string templ_img_path =
      "F:/DataSet/dataset2/template/template.png";
  const std::string tested_img_path = "F:/DataSet/dataset2/data/";

  const std::filesystem::path root_path =
      "F:/DataSet/dataset2/optflow-acpm13-epicflow/";

  // 写入统计指标
  std::ofstream outfile;
  outfile.open(metric_path, std::ios::out | std::ios::trunc);

  for (auto const &iter : std::filesystem::directory_iterator(root_path)) {

    if (iter.path().extension().string() == ".flo") {
      const std::string optflow_path = iter.path().string();
      const std::string tested_img_cur_path =
          tested_img_path + iter.path().stem().string() + ".png";
      const std::string res_img_cur_path =
          res_img_path + iter.path().stem().string() + ".png";

      std::cout << "optflow_path: " << optflow_path << std::endl;
      std::cout << "templ_img_path: " << templ_img_path << std::endl;
      std::cout << "tested_img_cur_path: " << tested_img_cur_path << std::endl;
      std::cout << "res_img_cur_path: " << res_img_cur_path << std::endl;

      cv::Mat templ_img = cv::imread(templ_img_path);
      cv::Mat tested_img = cv::imread(tested_img_cur_path);

      cv::cvtColor(templ_img, templ_img, cv::COLOR_BGR2GRAY);
      cv::cvtColor(tested_img, tested_img, cv::COLOR_BGR2GRAY);

      cv::Mat optflow;
      ReadFlowFile(optflow, optflow_path.c_str());

      std::cout << "templ_img size: " << templ_img.size() << std::endl;
      std::cout << "tested_img size: " << tested_img.size() << std::endl;
      std::cout << "optflow size: " << optflow.size() << std::endl;

      cv::Mat tested_img_warpped;
      MovePixels(tested_img, tested_img_warpped, optflow, cv::INTER_CUBIC);
      cv::imwrite(res_img_cur_path, cv::abs(templ_img - tested_img_warpped));

      float mse = MSE(templ_img, tested_img_warpped);
      float cc = CorrelationCoefficient(templ_img, tested_img_warpped);
      float nmi = NormalizedMutualInformation(templ_img, tested_img_warpped);
      float ssim = MSSIM(templ_img, tested_img_warpped);

      // cv::Mat templ_img_warpped;
      // MovePixels(templ_img, templ_img_warpped, optflow, cv::INTER_CUBIC);
      // cv::imwrite(res_img_cur_path, cv::abs(templ_img_warpped - tested_img));

      // float mse = MSE(templ_img_warpped, tested_img);
      // float cc = CorrelationCoefficient(templ_img_warpped, tested_img);
      // float nmi = NormalizedMutualInformation(templ_img_warpped, tested_img);
      // float ssim = MSSIM(templ_img_warpped, tested_img);

      outfile << mse << " " << cc << " " << nmi << " " << ssim << std::endl;

      // cv::imwrite(tested_img_warpped_path, tested_img_warpped);
      // cv::imwrite(tested_img_res_path, cv::abs(templ_img -
      // tested_img_warpped));
    }
  }

  outfile.close();

  return 0;
}