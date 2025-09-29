#!/bin/bash

# 脚本用于压缩microbenchmark中的Artificial_{n}.log文件
# 使用zstd压缩器，阈值为1，窗口大小为1
# 测试use_approx的true和false两种情况

# 设置输入和输出目录
input_dir="./exp_microbenchmark"
output_dir_approx="./result_microbenchmark/result_approx"
output_dir_exact="./result_microbenchmark/result_exact"

# 确保输出目录存在
mkdir -p "$output_dir_approx"
mkdir -p "$output_dir_exact"

# 压缩器参数
compressor="zstd"
window_size="1"
threshold="0.06"
block_size="100000000"  # 使用run_compression.sh中的默认值
distance="minhash"      # 使用minhash距离度量

# 创建CSV文件头
echo "n,Compression_Time" > "$output_dir_approx/compression_timing.csv"
echo "n,Compression_Time" > "$output_dir_exact/compression_timing.csv"

echo "开始处理microbenchmark数据..."
echo "输入目录: $input_dir"
echo "近似匹配输出目录: $output_dir_approx"
echo "精确匹配输出目录: $output_dir_exact"
echo "压缩器: $compressor"
echo "阈值: $threshold"
echo "窗口大小: $window_size"
echo ""

# 处理Artificial_3.log到Artificial_20.log
for n in {50..100}; do
    input_file="${input_dir}/Artificial_${n}.log"
    
    # 检查输入文件是否存在
    if [ -f "$input_file" ]; then
        echo "正在处理: $input_file"
        
        # 处理近似匹配 (use_approx=true)
        echo "  -> 近似匹配模式"
        output_file_approx="${output_dir_approx}/Artificial_${n}"
        echo "     输出到: $output_file_approx"
        
        # 记录开始时间
        start_time=$(date +%s.%N)
        
        # 运行压缩命令 (近似匹配)
        ./record_compress "$input_file" "$output_file_approx" "$compressor" "$window_size" "$threshold" "$block_size" "$distance" "true"
        
        # 记录结束时间并计算耗时
        end_time=$(date +%s.%N)
        compression_time=$(echo "$end_time - $start_time" | bc)
        
        # 检查压缩是否成功
        if [ $? -eq 0 ]; then
            echo "     ✓ 近似匹配成功，耗时: ${compression_time}秒"
            echo "${n},${compression_time}" >> "$output_dir_approx/compression_timing.csv"
        else
            echo "     ✗ 近似匹配处理失败"
        fi
        
        # 处理精确匹配 (use_approx=false)
        echo "  -> 精确匹配模式"
        output_file_exact="${output_dir_exact}/Artificial_${n}"
        echo "     输出到: $output_file_exact"
        
        # 记录开始时间
        start_time=$(date +%s.%N)
        
        # 运行压缩命令 (精确匹配)
        ./record_compress "$input_file" "$output_file_exact" "$compressor" "$window_size" "$threshold" "$block_size" "$distance" "false"
        
        # 记录结束时间并计算耗时
        end_time=$(date +%s.%N)
        compression_time=$(echo "$end_time - $start_time" | bc)
        
        # 检查压缩是否成功
        if [ $? -eq 0 ]; then
            echo "     ✓ 精确匹配成功，耗时: ${compression_time}秒"
            echo "${n},${compression_time}" >> "$output_dir_exact/compression_timing.csv"
        else
            echo "     ✗ 精确匹配处理失败"
        fi
        
        echo ""
    else
        echo "⚠ 文件不存在: $input_file"
    fi
done

echo "所有microbenchmark数据处理完成！"
echo "近似匹配结果保存在: $output_dir_approx"
echo "精确匹配结果保存在: $output_dir_exact"
echo "压缩时间记录在各自的compression_timing.csv文件中"
