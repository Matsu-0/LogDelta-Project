#!/bin/bash

# 脚本用于运行 record_compress 处理 Android 数据集
# 测试不同 block size 的压缩效果
# 结果输出到 result_block_size/Android/
# 压缩时间保存到 time_cost.csv

# 定义数据集（只测试 Android）
datasets=("Android")

# 定义 Android 数据集的最优参数（来自 exp1_whole_performace.sh）
declare -A thresholds=(
    ["Android"]="0.10"
)

declare -A q_values=(
    ["Android"]="11"
)

# 定义 block size 序列：100, 200, 400, ..., 51200（每次乘以2）和 100000
block_sizes=(100 200 400 800 1600 3200 6400 12800 25600 51200 100000)

# 创建结果根目录
mkdir -p ./result_block_size

# 检查 record_compress 可执行文件是否存在
if [ ! -f "./record_compress" ]; then
    echo "Error: record_compress executable not found!"
    echo "Please make sure record_compress is in the current directory"
    exit 1
fi

# 创建压缩时间CSV文件
time_csv="./result_block_size/time_cost.csv"

# 创建CSV头部（第一行：Dataset, 100, 200, 400, ..., 100000）
echo -n "Dataset" > "$time_csv"
for block_size in "${block_sizes[@]}"; do
    echo -n ",${block_size}" >> "$time_csv"
done
echo "" >> "$time_csv"

# 处理 Android 数据集
for dataset in "${datasets[@]}"; do
    echo "=========================================="
    echo "Processing dataset: $dataset"
    echo "=========================================="
    
    # 构建输入文件路径
    input_file="./whole_file/${dataset}.log"
    
    # 检查输入文件是否存在
    if [ ! -f "$input_file" ]; then
        echo "Warning: Input file $input_file not found, skipping..."
        echo "${dataset},N/A,N/A,N/A,N/A,N/A,N/A,N/A,N/A,N/A,N/A,N/A" >> "$time_csv"
        continue
    fi
    
    # 获取该数据集的阈值和Q值
    threshold="${thresholds[$dataset]}"
    q_value="${q_values[$dataset]}"
    
    echo "Input file: $input_file"
    echo "Threshold: $threshold"
    echo "Q value: $q_value"
    echo ""
    
    # 开始该数据集的行
    echo -n "${dataset}" >> "$time_csv"
    
    # 测试每个 block size
    for block_size in "${block_sizes[@]}"; do
        echo "  Testing block size: $block_size"
        
        # 构建输出文件路径
        output_file="./result_block_size/Android/Android_${block_size}"
        
        echo "    Output file: $output_file"
        echo "    Running compression..."
        
        # 运行 record_compress 并捕获输出以提取压缩时间
        # 参数: input_path output_path compressor window_size threshold block_size distance use_approx q_value
        # 使用 Android 的最优参数：threshold=0.10, q_value=11, window_size=8
        output_log=$(./record_compress "$input_file" "$output_file" "lzma" 8 "$threshold" "$block_size" "minhash" "true" "$q_value" 2>&1)
        
        # 检查命令是否成功执行
        if [ $? -eq 0 ]; then
            echo "    ✓ Successfully processed ${dataset} with block size ${block_size}"
            
            # 从输出中提取压缩时间
            compression_time=$(echo "$output_log" | grep "Compressing Total time" | grep -o '[0-9.]*' | head -1)
            if [ -n "$compression_time" ]; then
                echo "    Compression time: ${compression_time} seconds"
                echo -n ",${compression_time}" >> "$time_csv"
            else
                echo "    Warning: Could not extract compression time from output"
                echo -n ",N/A" >> "$time_csv"
            fi
        else
            echo "    ✗ Error processing ${dataset} with block size ${block_size}"
            echo -n ",N/A" >> "$time_csv"
        fi
        
        echo ""
    done
    
    echo "" >> "$time_csv"
    echo "Completed all block sizes for $dataset"
    echo ""
done

echo "=========================================="
echo "All block size testing completed!"
echo "=========================================="

# 显示结果统计
echo "Generated files by block size:"
for block_size in "${block_sizes[@]}"; do
    output_file="./result_block_size/Android/Android_${block_size}"
    if [ -f "$output_file" ]; then
        echo "  Block size ${block_size}: ✓ Compressed file generated"
    else
        echo "  Block size ${block_size}: ✗ No compressed file found"
    fi
done

echo ""
echo "Total compressed files generated:"
ls -1 ./result_block_size/Android/Android_* 2>/dev/null | wc -l

echo ""
echo "Files in result_block_size directory:"
ls -la ./result_block_size/

echo ""
echo "=========================================="
echo "Compression time summary:"
echo "=========================================="

# 显示压缩时间CSV内容
echo "Time cost CSV content:"
cat "$time_csv"

echo ""
echo "Summary statistics:"
echo "Total block sizes: ${#block_sizes[@]}"
echo "Successful compressions: $(grep -o '[0-9.]*' "$time_csv" | grep -v '^[0-9]*$' | wc -l)"
echo "Failed compressions: $(grep -c "N/A" "$time_csv")"

echo ""
echo "All processing completed! Check $time_csv for compression times."
