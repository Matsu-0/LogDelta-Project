#!/bin/bash

# 脚本用于运行 record_compress 处理 exp_reorder 中的文件
# 测试不同日志顺序的压缩效果
# 结果输出到 result_log_order/[dataset]/
# 压缩时间保存到 time_cost.csv

# 定义数据集列表
datasets=("OpenStack" "Spark" "Zookeeper")

# 定义日志顺序类型
log_orders=("original" "restructured" "shuffled")

# 定义每个数据集的阈值（使用默认阈值）
declare -A thresholds=(
    ["OpenStack"]="0.02"
    ["Spark"]="0.02"
    ["Zookeeper"]="0.26"
)

# 定义每个数据集的Q值（使用默认Q值）
declare -A q_values=(
    ["OpenStack"]="3"
    ["Spark"]="4"
    ["Zookeeper"]="13"
)

# 创建结果根目录
mkdir -p ./result_log_order

# 检查 record_compress 可执行文件是否存在
if [ ! -f "./record_compress" ]; then
    echo "Error: record_compress executable not found!"
    echo "Please make sure record_compress is in the current directory"
    exit 1
fi

# 创建压缩时间CSV文件
time_csv="./result_log_order/time_cost.csv"

# 创建CSV头部（第一行：Dataset, original, restructured, shuffled）
echo -n "Dataset" > "$time_csv"
for log_order in "${log_orders[@]}"; do
    echo -n ",${log_order}" >> "$time_csv"
done
echo "" >> "$time_csv"

# 处理每个数据集
for dataset in "${datasets[@]}"; do
    echo "=========================================="
    echo "Processing dataset: $dataset"
    echo "=========================================="
    
    # 获取该数据集的阈值和Q值
    threshold="${thresholds[$dataset]}"
    q_value="${q_values[$dataset]}"
    
    echo "Threshold: $threshold"
    echo "Q value: $q_value"
    echo ""
    
    # 为该数据集创建目录
    dataset_dir="./result_log_order/${dataset}"
    mkdir -p "$dataset_dir"
    
    # 开始该数据集的行
    echo -n "${dataset}" >> "$time_csv"
    
    # 测试每种日志顺序
    for log_order in "${log_orders[@]}"; do
        echo "  Testing log order: $log_order"
        
        # 构建输入文件路径
        if [ "$log_order" = "original" ]; then
            input_file="./exp_reorder/${dataset}.log"
        elif [ "$log_order" = "restructured" ]; then
            input_file="./exp_reorder/${dataset}_restructured.log"
        elif [ "$log_order" = "shuffled" ]; then
            input_file="./exp_reorder/${dataset}_shuffled.log"
        fi
        
        # 检查输入文件是否存在
        if [ ! -f "$input_file" ]; then
            echo "    Warning: Input file $input_file not found, skipping..."
            echo -n ",N/A" >> "$time_csv"
            continue
        fi
        
        # 构建输出文件路径
        output_file="${dataset_dir}/${dataset}_${log_order}"
        
        echo "    Input file: $input_file"
        echo "    Output file: $output_file"
        echo "    Running compression..."
        
        # 运行 record_compress 并捕获输出以提取压缩时间
        # 参数: input_path output_path compressor window_size threshold block_size distance use_approx q_value
        # 使用默认参数：compressor=lzma, window_size=8, block_size=100000000, distance=minhash, use_approx=true
        output_log=$(./record_compress "$input_file" "$output_file" "lzma" 8 "$threshold" 100000000 "minhash" "true" "$q_value" 2>&1)
        
        # 检查命令是否成功执行
        if [ $? -eq 0 ]; then
            echo "    ✓ Successfully processed ${dataset} with ${log_order} order"
            
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
            echo "    ✗ Error processing ${dataset} with ${log_order} order"
            echo -n ",N/A" >> "$time_csv"
        fi
        
        echo ""
    done
    
    echo "" >> "$time_csv"
    echo "Completed all log orders for $dataset"
    echo ""
done

echo "=========================================="
echo "All log order testing completed!"
echo "=========================================="

# 显示结果统计
echo "Generated files by dataset:"
for dataset in "${datasets[@]}"; do
    dataset_dir="./result_log_order/${dataset}"
    if [ -d "$dataset_dir" ]; then
        file_count=$(ls -1 "$dataset_dir"/* 2>/dev/null | wc -l)
        echo "  $dataset: $file_count files"
    fi
done

echo ""
echo "Total compressed files generated:"
find ./result_log_order -name "*" -type f | wc -l

echo ""
echo "Directory structure:"
tree ./result_log_order 2>/dev/null || find ./result_log_order -type d | sort

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
echo "Total log orders per dataset: ${#log_orders[@]}"
echo "Total experiments: $((${#datasets[@]} * ${#log_orders[@]}))"
echo "Successful compressions: $(grep -o '[0-9.]*' "$time_csv" | grep -v '^[0-9]*$' | wc -l)"
echo "Failed compressions: $(grep -c "N/A" "$time_csv")"

echo ""
echo "All processing completed! Check $time_csv for compression times."
