#!/bin/bash

# 脚本用于运行 record_compress_new 处理10个数据集
# 每个数据集使用特定的阈值和Q值

# 定义数据集列表
datasets=("Android" "Apache" "Mac" "OpenStack" "Spark" "Zookeeper" "SSH" "Linux" "Proxifier" "Thunderbird")

# datasets=("Linux" "Apache")

# 定义每个数据集的阈值
declare -A thresholds=(
    ["Android"]="0.10"
    ["Apache"]="0.02"
    ["Mac"]="0.02"
    ["OpenStack"]="0.02"
    ["Spark"]="0.02"
    ["Zookeeper"]="0.26"
    ["SSH"]="0.18"
    ["Linux"]="0.06"
    ["Proxifier"]="0.02"
    ["Thunderbird"]="0.12"
)

# 定义每个数据集的最优Q值
declare -A q_values=(
    ["Android"]="11"
    ["Apache"]="3"
    ["Mac"]="11"
    ["OpenStack"]="3"
    ["Spark"]="4"
    ["Zookeeper"]="13"
    ["SSH"]="21"
    ["Linux"]="10"
    ["Proxifier"]="4"
    ["Thunderbird"]="4"
)

# 创建结果目录
mkdir -p ./result_split_column

# 检查 record_compress_new 可执行文件是否存在
if [ ! -f "./record_compress_new" ]; then
    echo "Error: record_compress_new executable not found!"
    echo "Please make sure record_compress_new is in the current directory"
    exit 1
fi

# 处理每个数据集
for dataset in "${datasets[@]}"; do
    echo "=========================================="
    echo "Processing dataset: $dataset"
    echo "=========================================="
    
    # 构建输入文件路径
    input_file="./whole_file/${dataset}.log"
    
    # 检查输入文件是否存在
    if [ ! -f "$input_file" ]; then
        echo "Warning: Input file $input_file not found, skipping..."
        continue
    fi
    
    # 获取该数据集的阈值和Q值
    threshold="${thresholds[$dataset]}"
    q_value="${q_values[$dataset]}"
    
    # 构建输出文件路径
    output_file="./result_split_column/${dataset}_output"
    
    echo "Input file: $input_file"
    echo "Output file: $output_file"
    echo "Threshold: $threshold"
    echo "Q value: $q_value"
    echo "Running compression..."
    
    # 运行 record_compress_new
    # 参数: input_path output_path compressor window_size threshold block_size distance use_approx q_value
    ./record_compress_new "$input_file" "$output_file" "none" 8 "$threshold" 100000000 "minhash" "true" "$q_value"
    
    # 检查命令是否成功执行
    if [ $? -eq 0 ]; then
        echo "Successfully processed $dataset"
        
        # 检查生成的CSV文件
        csv_file="${output_file}_records.csv"
        if [ -f "$csv_file" ]; then
            echo "CSV file generated: $csv_file"
            # 显示CSV文件的前几行
            echo "First 5 lines of CSV:"
            head -5 "$csv_file"
            echo ""
        else
            echo "Warning: CSV file not found: $csv_file"
        fi
    else
        echo "Error processing $dataset"
    fi
    
    echo ""
done

echo "=========================================="
echo "All datasets processing completed!"
echo "=========================================="

# 显示结果统计
echo "Generated files:"
ls -la ./result_split_column/*.csv 2>/dev/null | wc -l | xargs echo "Total CSV files:"
echo ""
echo "File sizes:"
ls -lh ./result_split_column/*.csv 2>/dev/null | awk '{print $5, $9}' | sort -hr
