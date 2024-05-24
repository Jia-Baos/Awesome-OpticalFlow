#include <iostream>
#include <string>
#include <fstream>
#include "./evalFunc.hpp"
#include "./include/opencv2/opencv.hpp"

int main(int argc, char *argv[])
{
    std::cout << "Eval begin..." << std::endl;

    std::string optflow_groundtruth_path = "E:/DataSets/flow/frame1/frame_0029.flo";
    std::string optflow_ACPMFlow_path = "E:/DataSets/estimate/ACPMFlow/frame1/frame_0029.flo";
    std::string optflow_CPMFlow_path = "E:/DataSets/estimate/CPMFlow/frame1/frame_0029.flo";
    std::string optflow_DeepFlow_path = "E:/DataSets/estimate/DeepFlow/frame1/frame_0029.flo";
    std::string optflow_EpicFlow_path = "E:/DataSets/estimate/EpicFlow/frame1/frame_0029.flo";
    std::string optflow_LDOF_path = "E:/DataSets/estimate/LDOF/frame1/frame_0029.flo";
    std::string optflow_RicFlow_path = "E:/DataSets/estimate/RicFlow/frame1/frame_0029.flo";

    cv::Mat optflow_groundtruth;
    cv::Mat optflow_ACPMFlow;
    cv::Mat optflow_CPMFlow;
    cv::Mat optflow_DeepFlow;
    cv::Mat optflow_EpicFlow;
    cv::Mat optflow_LDOF;
    cv::Mat optflow_RicFlow;

    ReadFlowFile(optflow_groundtruth, optflow_groundtruth_path.c_str());
    ReadFlowFile(optflow_ACPMFlow, optflow_ACPMFlow_path.c_str());
    ReadFlowFile(optflow_CPMFlow, optflow_CPMFlow_path.c_str());
    ReadFlowFile(optflow_DeepFlow, optflow_DeepFlow_path.c_str());
    ReadFlowFile(optflow_EpicFlow, optflow_EpicFlow_path.c_str());
    ReadFlowFile(optflow_LDOF, optflow_LDOF_path.c_str());
    ReadFlowFile(optflow_RicFlow, optflow_RicFlow_path.c_str());

    int R1 = 0;
    int R2 = 0;
    int R3 = 0;
    float aee = 0;
    cv::Mat optflow_err;

    std::cout << "optflow_ACPM: " << std::endl;
    optflow_err = endpointError(optflow_ACPMFlow, optflow_groundtruth);
    aee = AEE(optflow_err);
    R1 = stat_RX(optflow_err, 1);
    R2 = stat_RX(optflow_err, 2);
    R3 = stat_RX(optflow_err, 3);
    std::cout << "aee: " << aee
              << " R1: " << R1
              << " R2: " << R2
              << " R3: " << R3 << std::endl;

    std::cout << "optflow_CPM: " << std::endl;
    optflow_err = endpointError(optflow_CPMFlow, optflow_groundtruth);
    aee = AEE(optflow_err);
    R1 = stat_RX(optflow_err, 1);
    R2 = stat_RX(optflow_err, 2);
    R3 = stat_RX(optflow_err, 3);
    std::cout << "aee: " << aee
              << " R1: " << R1
              << " R2: " << R2
              << " R3: " << R3 << std::endl;

    std::cout << "optflow_DeepFlow: " << std::endl;
    optflow_err = endpointError(optflow_DeepFlow, optflow_groundtruth);
    aee = AEE(optflow_err);
    R1 = stat_RX(optflow_err, 1);
    R2 = stat_RX(optflow_err, 2);
    R3 = stat_RX(optflow_err, 3);
    std::cout << "aee: " << aee
              << " R1: " << R1
              << " R2: " << R2
              << " R3: " << R3 << std::endl;

    std::cout << "optflow_EpicFlow: " << std::endl;
    optflow_err = endpointError(optflow_EpicFlow, optflow_groundtruth);
    aee = AEE(optflow_err);
    R1 = stat_RX(optflow_err, 1);
    R2 = stat_RX(optflow_err, 2);
    R3 = stat_RX(optflow_err, 3);
    std::cout << "aee: " << aee
              << " R1: " << R1
              << " R2: " << R2
              << " R3: " << R3 << std::endl;

    std::cout << "optflow_LDOF: " << std::endl;
    optflow_err = endpointError(optflow_LDOF, optflow_groundtruth);
    aee = AEE(optflow_err);
    R1 = stat_RX(optflow_err, 1);
    R2 = stat_RX(optflow_err, 2);
    R3 = stat_RX(optflow_err, 3);
    std::cout << "aee: " << aee
              << " R1: " << R1
              << " R2: " << R2
              << " R3: " << R3 << std::endl;

    std::cout << "optflow_RicFlow: " << std::endl;
    optflow_err = endpointError(optflow_RicFlow, optflow_groundtruth);
    aee = AEE(optflow_err);
    R1 = stat_RX(optflow_err, 1);
    R2 = stat_RX(optflow_err, 2);
    R3 = stat_RX(optflow_err, 3);
    std::cout << "aee: " << aee
              << " R1: " << R1
              << " R2: " << R2
              << " R3: " << R3 << std::endl;

    std::cout << "Eval end..." << std::endl;

    return 0;
}
