#!/bin/bash

# 脚本用于运行 record_compress 处理10个数据集
# 测试不同的窗口大小：1, 2, 4, 8, 16, 32, 64, 128, 256
# 结果按数据集分类输出到 result_window_size/[dataset]/[dataset]_{n}

# 定义数据集列表
datasets=("Android" "Apache" "Mac" "OpenStack" "Spark" "Zookeeper" "SSH" "Linux" "Proxifier" "Thunderbird")

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

# 定义窗口大小序列：1, 2, 4, 8, 16, 32, 64, 128, 256
window_sizes=(1 2 4 8 16 32 64 128 256)

# 创建结果根目录
mkdir -p ./result_window_size

# 检查 record_compress 可执行文件是否存在
if [ ! -f "./record_compress" ]; then
    echo "Error: record_compress executable not found!"
    echo "Please make sure record_compress is in the current directory"
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
    
    # 为该数据集创建目录
    dataset_dir="./result_window_size/${dataset}"
    mkdir -p "$dataset_dir"
    
    echo "Input file: $input_file"
    echo "Threshold: $threshold"
    echo "Q value: $q_value"
    echo "Dataset directory: $dataset_dir"
    echo ""
    
    # 测试每个窗口大小
    for window_size in "${window_sizes[@]}"; do
        echo "  Testing window size: $window_size"
        
        # 构建输出文件路径
        output_file="${dataset_dir}/${dataset}_${window_size}"
        
        echo "    Output file: $output_file"
        echo "    Running compression..."
        
        # 运行 record_compress 并将输出重定向到日志文件
        # 参数: input_path output_path compressor window_size threshold block_size distance use_approx q_value
        ./record_compress "$input_file" "$output_file" "lzma" "$window_size" "$threshold" 100000000 "minhash" "true" "$q_value" > "${output_file}.log" 2>&1
        
        # 检查命令是否成功执行
        if [ $? -eq 0 ]; then
            echo "    ✓ Successfully processed ${dataset} with window size ${window_size}"
        else
            echo "    ✗ Error processing ${dataset} with window size ${window_size}"
        fi
        
        echo ""
    done
    
    echo "Completed all window sizes for $dataset"
    echo ""
done

echo "=========================================="
echo "All datasets processing completed!"
echo "=========================================="

# 显示结果统计
echo "Generated files by dataset:"
for dataset in "${datasets[@]}"; do
    dataset_dir="./result_window_size/${dataset}"
    if [ -d "$dataset_dir" ]; then
        csv_count=$(ls -1 "$dataset_dir"/*.csv 2>/dev/null | wc -l)
        echo "  $dataset: $csv_count CSV files"
    fi
done

echo ""
echo "Total CSV files generated:"
find ./result_window_size -name "*.csv" | wc -l

echo ""
echo "Directory structure:"
tree ./result_window_size 2>/dev/null || find ./result_window_size -type d | sort

echo ""
echo "=========================================="
echo "Generating compression time matrix CSV..."
echo "=========================================="

# 创建压缩时间矩阵CSV文件
summary_csv="./result_window_size/compression_time_matrix.csv"

# 创建CSV头部（第一行：Dataset, 1, 2, 4, 8, 16, 32, 64, 128, 256）
echo -n "Dataset" > "$summary_csv"
for window_size in "${window_sizes[@]}"; do
    echo -n ",${window_size}" >> "$summary_csv"
done
echo "" >> "$summary_csv"

# 为每个数据集生成一行数据
for dataset in "${datasets[@]}"; do
    dataset_dir="./result_window_size/${dataset}"
    
    # 开始该数据集的行
    echo -n "${dataset}" >> "$summary_csv"
    
    for window_size in "${window_sizes[@]}"; do
        output_file="${dataset_dir}/${dataset}_${window_size}"
        log_file="${output_file}.log"
        
        # 尝试从日志文件中提取压缩时间
        if [ -f "$log_file" ]; then
            # 查找包含"Compressing Total time"的行并提取时间
            compression_time=$(grep "Compressing Total time" "$log_file" 2>/dev/null | grep -o '[0-9.]*' | head -1)
            if [ -n "$compression_time" ]; then
                echo -n ",${compression_time}" >> "$summary_csv"
            else
                echo -n ",N/A" >> "$summary_csv"
            fi
        else
            echo -n ",N/A" >> "$summary_csv"
        fi
    done
    
    echo "" >> "$summary_csv"
done

echo "✓ Compression time matrix CSV generated: $summary_csv"

# 显示汇总统计
echo ""
echo "Summary statistics:"
echo "Total experiments: $((${#datasets[@]} * ${#window_sizes[@]}))"
echo "Successful experiments: $(grep -o '[0-9.]*' "$summary_csv" | grep -v '^[0-9]*$' | wc -l)"
echo "Failed experiments: $(grep -c "N/A" "$summary_csv")"

echo ""
echo "Matrix preview (first 5 rows):"
head -6 "$summary_csv"

echo ""
echo "All processing completed! Check $summary_csv for compression time matrix."
