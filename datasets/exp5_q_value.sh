#!/bin/bash

# 脚本用于运行 record_compress 处理 whole_file 中的文件
# 测试不同 q_value 的压缩效果
# 结果输出到 result_q_value/[dataset]/
# 压缩时间保存到 time_cost.csv

# 定义数据集列表
datasets=("Android" "Apache" "Mac" "OpenStack" "Spark" "Zookeeper" "SSH" "Linux" "Proxifier" "Thunderbird")

# 定义每个数据集的阈值（来自 exp1_whole_performace.sh）
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

# 定义 q_value 范围：从1到100
q_values=()
for q in {1..100}; do
    q_values+=($q)
done

# 创建结果根目录
mkdir -p ./result_q_value

# 检查 record_compress 可执行文件是否存在
if [ ! -f "./record_compress" ]; then
    echo "Error: record_compress executable not found!"
    echo "Please make sure record_compress is in the current directory"
    exit 1
fi

# 创建压缩时间CSV文件
time_csv="./result_q_value/time_cost.csv"

# 创建CSV头部（第一行：Dataset, 1, 2, 3, ..., 100）
echo -n "Dataset" > "$time_csv"
for q_value in "${q_values[@]}"; do
    echo -n ",${q_value}" >> "$time_csv"
done
echo "" >> "$time_csv"

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
        # 为该数据集添加一行N/A数据
        echo -n "${dataset}" >> "$time_csv"
        for q_value in "${q_values[@]}"; do
            echo -n ",N/A" >> "$time_csv"
        done
        echo "" >> "$time_csv"
        continue
    fi
    
    # 获取该数据集的阈值
    threshold="${thresholds[$dataset]}"
    
    # 为该数据集创建目录
    dataset_dir="./result_q_value/${dataset}"
    mkdir -p "$dataset_dir"
    
    echo "Input file: $input_file"
    echo "Threshold: $threshold"
    echo "Dataset directory: $dataset_dir"
    echo ""
    
    # 开始该数据集的行
    echo -n "${dataset}" >> "$time_csv"
    
    # 测试每个 q_value
    for q_value in "${q_values[@]}"; do
        echo "  Testing q_value: $q_value"
        
        # 构建输出文件路径
        output_file="${dataset_dir}/${dataset}_${q_value}"
        
        echo "    Output file: $output_file"
        echo "    Running compression..."
        
        # 运行 record_compress 并捕获输出以提取压缩时间
        # 参数: input_path output_path compressor window_size threshold block_size distance use_approx q_value
        # 使用默认参数：compressor=lzma, window_size=8, block_size=100000000, distance=minhash, use_approx=true
        output_log=$(./record_compress "$input_file" "$output_file" "lzma" 8 "$threshold" 100000000 "minhash" "true" "$q_value" 2>&1)
        
        # 检查命令是否成功执行
        if [ $? -eq 0 ]; then
            echo "    ✓ Successfully processed ${dataset} with q_value ${q_value}"
            
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
            echo "    ✗ Error processing ${dataset} with q_value ${q_value}"
            echo -n ",N/A" >> "$time_csv"
        fi
        
        echo ""
    done
    
    echo "" >> "$time_csv"
    echo "Completed all q_values for $dataset"
    echo ""
done

echo "=========================================="
echo "All q_value testing completed!"
echo "=========================================="

# 显示结果统计
echo "Generated files by dataset:"
for dataset in "${datasets[@]}"; do
    dataset_dir="./result_q_value/${dataset}"
    if [ -d "$dataset_dir" ]; then
        file_count=$(ls -1 "$dataset_dir"/* 2>/dev/null | wc -l)
        echo "  $dataset: $file_count files"
    fi
done

echo ""
echo "Total compressed files generated:"
find ./result_q_value -name "*" -type f | wc -l

echo ""
echo "Directory structure:"
tree ./result_q_value 2>/dev/null || find ./result_q_value -type d | sort

echo ""
echo "=========================================="
echo "Compression time summary:"
echo "=========================================="

# 显示压缩时间CSV内容（只显示前几行）
echo "Time cost CSV content (first 5 rows):"
head -6 "$time_csv"

echo ""
echo "Summary statistics:"
echo "Total datasets: ${#datasets[@]}"
echo "Total q_values per dataset: ${#q_values[@]}"
echo "Total experiments: $((${#datasets[@]} * ${#q_values[@]}))"
echo "Successful compressions: $(grep -o '[0-9.]*' "$time_csv" | grep -v '^[0-9]*$' | wc -l)"
echo "Failed compressions: $(grep -c "N/A" "$time_csv")"

echo ""
echo "All processing completed! Check $time_csv for compression times."
