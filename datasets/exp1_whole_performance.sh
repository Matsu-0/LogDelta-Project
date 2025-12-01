#!/bin/bash

# 脚本用于运行 record_compress 处理10个数据集
# 测试 approx 压缩效果
# 结果输出到 result_whole_file/[dataset]/
# 压缩时间保存到 time_cost.csv

# 定义数据集列表
datasets=("Android" "Apache" "Mac" "OpenStack" "Spark" "Zookeeper" "SSH" "Linux" "Proxifier" "Thunderbird")

# 定义每个数据集的阈值
# declare -A thresholds=(
#     ["Android"]="0.10"
#     ["Apache"]="0.02"
#     ["Mac"]="0.02"
#     ["OpenStack"]="0.02"
#     ["Spark"]="0.02"
#     ["Zookeeper"]="0.26"
#     ["SSH"]="0.18"
#     ["Linux"]="0.06"
#     ["Proxifier"]="0.02"
#     ["Thunderbird"]="0.12"
# )

declare -A thresholds=(
    ["Android"]="0.01"
    ["Apache"]="0.02"
    ["Mac"]="0.02"
    ["OpenStack"]="0.02"
    ["Spark"]="0.01"
    ["Zookeeper"]="0.26"
    ["SSH"]="0.18"
    # ["Linux"]="0.01"
    ["Linux"]="0.06"
    ["Proxifier"]="0.02"
    ["Thunderbird"]="0.02"
)

# # 定义每个数据集的最优Q值
# declare -A q_values=(
#     ["Android"]="11"
#     ["Apache"]="3"
#     ["Mac"]="11"
#     ["OpenStack"]="3"
#     ["Spark"]="4"
#     ["Zookeeper"]="13"
#     ["SSH"]="21"
#     ["Linux"]="10"
#     ["Proxifier"]="4"
#     ["Thunderbird"]="4"
# )


declare -A q_values=(
    ["Android"]="9"
    ["Apache"]="3"
    ["Mac"]="13"
    ["OpenStack"]="3"
    ["Spark"]="2"
    ["Zookeeper"]="13"
    ["SSH"]="21"
    # ["Linux"]="17"
    ["Linux"]="10"
    # ["Proxifier"]="11"
    ["Proxifier"]="4"
    ["Thunderbird"]="12"
)

# 定义每个数据集的压缩器类型
# 支持的压缩器: none, lzma, gzip, zstd, lz4, bzip2
declare -A compressors=(
    ["Android"]="zstd"
    ["Apache"]="zstd"
    ["Mac"]="zstd"
    ["OpenStack"]="zstd"
    ["Spark"]="zstd"
    ["Zookeeper"]="lzma"
    ["SSH"]="zstd"
    ["Linux"]="lzma"
    ["Proxifier"]="lzma"
    ["Thunderbird"]="zstd"
)


# 创建结果根目录
mkdir -p ./result_whole_file

# 检查 record_compress 可执行文件是否存在
if [ ! -f "./record_compress" ]; then
    echo "Error: record_compress executable not found!"
    echo "Please make sure record_compress is in the current directory"
    exit 1
fi

# 创建压缩时间CSV文件
time_csv="./result_whole_file/time_cost.csv"
echo "Dataset,Compression_Time" > "$time_csv"

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
        echo "${dataset},N/A" >> "$time_csv"
        continue
    fi
    
    # 获取该数据集的阈值、Q值和压缩器类型
    threshold="${thresholds[$dataset]}"
    q_value="${q_values[$dataset]}"
    compressor="${compressors[$dataset]}"
    
    # 如果未定义压缩器，使用默认值 zstd
    if [ -z "$compressor" ]; then
        echo "  Warning: No compressor defined for ${dataset}, using default zstd"
        compressor="zstd"
    fi
    
    # 构建输出文件路径（直接保存在result_whole_file目录下）
    output_file="./result_whole_file/${dataset}"
    
    echo "Input file: $input_file"
    echo "Threshold: $threshold"
    echo "Q value: $q_value"
    echo "Compressor: $compressor"
    echo "Output file: $output_file"
    echo "Running approx compression..."
    
    # 运行 record_compress 并捕获输出以提取压缩时间
    # 参数: input_path output_path compressor window_size threshold block_size distance use_approx q_value
    # 使用窗口大小 8，approx 压缩
    output_log=$(./record_compress "$input_file" "$output_file" "$compressor" 8 "$threshold" 100000000 "minhash" "true" "$q_value" 2>&1)
    
    # 检查命令是否成功执行
    if [ $? -eq 0 ]; then
        echo "✓ Successfully processed ${dataset} with approx compression"
        
        # 从输出中提取压缩时间
        compression_time=$(echo "$output_log" | grep "Compressing Total time" | grep -o '[0-9.]*' | head -1)
        if [ -n "$compression_time" ]; then
            echo "  Compression time: ${compression_time} seconds"
            echo "${dataset},${compression_time}" >> "$time_csv"
        else
            echo "  Warning: Could not extract compression time from output"
            echo "${dataset},N/A" >> "$time_csv"
        fi
    else
        echo "✗ Error processing ${dataset} with approx compression"
        echo "${dataset},N/A" >> "$time_csv"
    fi
    
    echo ""
done

echo "=========================================="
echo "All datasets processing completed!"
echo "=========================================="

# 显示结果统计
echo "Generated compressed files:"
for dataset in "${datasets[@]}"; do
    compressor="${compressors[$dataset]}"
    if [ -z "$compressor" ]; then
        compressor="zstd"
    fi
    output_file="./result_whole_file/${dataset}.${compressor}"
    if [ -f "$output_file" ]; then
        echo "  $dataset: ✓ Compressed file generated (${compressor})"
    else
        echo "  $dataset: ✗ No compressed file found (expected: ${output_file})"
    fi
done

echo ""
echo "Total compressed files generated:"
find ./result_whole_file -name "*.lzma" -o -name "*.gzip" -o -name "*.zstd" -o -name "*.lz4" -o -name "*.bz2" -o -name "*.bin" 2>/dev/null | wc -l

echo ""
echo "Files in result_whole_file directory:"
ls -la ./result_whole_file/

echo ""
echo "=========================================="
echo "Compression time summary:"
echo "=========================================="

# 显示压缩时间CSV内容
echo "Time cost CSV content:"
cat "$time_csv"

echo ""
echo "Summary statistics:"
echo "Total datasets: ${#datasets[@]}"
echo "Successful compressions: $(grep -v "N/A" "$time_csv" | grep -v "Dataset" | wc -l)"
echo "Failed compressions: $(grep -c "N/A" "$time_csv")"

echo ""
echo "All processing completed! Check $time_csv for compression times."
