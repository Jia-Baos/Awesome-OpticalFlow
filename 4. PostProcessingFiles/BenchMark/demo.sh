#!/bin/bash

Data_folder="E:/DataSets/data"                          # 数据集路径
Method_folder="D:/Code-VSCode/Methods"     # 光流方法路径
Optflow_folder="E:/DataSets/estimate"                   # 光流结果路径

method_arr=(CPMFlow)  # 光流计算方法 arr
# method_arr=(ACPMFlow CPMFlow DeepFlow EpicFlow LDOF RicFlow)   # 光流计算方法数组

# 遍历数据类别目录，获取所有类别
cls_list=$(ls $Data_folder)
for cls_name in ${cls_list}
do
    echo "step1: traverse all classes of data"
    echo "  cls_name: ${cls_name}"

    index=0
    declare -a frame_arr
    frame_folder=${Data_folder}"/"${cls_name}
    # 遍历数据目录，获取所有帧的名称，并存于数组 frame_arr 中
    frame_list=$(ls $frame_folder)
    for frame_name in ${frame_list}
    do
        frame_arr[${index}]=${frame_name}
        index=$((${index}+1))
    done

    # 将数组 frame_arr 中连续的两帧提取出来，并构造各种方法的输入和输出
    frame_pair_num=$((${#frame_arr[*]}-1))
    for ((i=0; i<${frame_pair_num}; i++))
    do
        pre_frame_name=${frame_arr[${i}]}
        cur_frame_name=${frame_arr[${i}+1]}
        optflow_name=${pre_frame_name/%.png/.flo}
        pre_frame_path=${frame_folder}"/"${pre_frame_name}
        cur_frame_path=${frame_folder}"/"${cur_frame_name}
        
        echo "step2: traverse the frame_pair"
        echo "  pre_frame_path: ${pre_frame_path}"
        echo "  cur_frame_path: ${cur_frame_path}"

        # 遍历 method_arr 中的各种光流计算方法
        method_arr_length=$((${#method_arr[*]}))
        for ((j=0; j<${method_arr_length}; j++))
        do
            method_name=${method_arr[${j}]}
            echo "step3: traverse the optflow method"
            echo "  method_name: ${method_name}"

            # 判断 estimate 文件夹是否存在
            if [ ! -d "${Optflow_folder}" ];then
                mkdir "${Optflow_folder}"
            fi
            # 判断 method 文件夹是否存在
            optflow_path=${Optflow_folder}"/"${method_name}
            if [ ! -d "${optflow_path}" ];then
                mkdir "${optflow_path}"
            fi
            # 判断 cls_name 文件夹是否存在
            optflow_path=${Optflow_folder}"/"${method_name}"/"${cls_name}
            if [ ! -d "${optflow_path}" ];then
                mkdir "${optflow_path}"
            fi
            # 计算结果——光流文件路径
            optflow_path=${Optflow_folder}"/"${method_name}"/"${cls_name}"/"${optflow_name}
            # 计算光流方法脚本的路径
            method_path=${Method_folder}"/"${method_name}"/demo.sh"

            echo "  method_path: ${method_path}"
            echo "  optflow_path: ${optflow_path}"
            sh ${method_path} ${pre_frame_path} ${cur_frame_path} ${optflow_path}
        done
    done
done