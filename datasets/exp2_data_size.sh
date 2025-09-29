#!/bin/bash

# 脚本用于运行 record_compress 处理 exp_data_size 中的文件
# 测试不同数据大小的压缩效果
# 结果按数据集分类输出到 result_data_size/[dataset]/
# 使用默认参数进行压缩

# 定义数据集列表
datasets=("Android" "Apache" "HPC" "Linux" "Mac" "OpenStack" "Proxifier" "Spark" "SSH" "Thunderbird" "Zookeeper")

# 定义数据大小序列：从1000到20000，步长500
data_sizes=(1000 1500 2000 2500 3000 3500 4000 4500 5000 5500 6000 6500 7000 7500 8000 8500 9000 9500 10000 10500 11000 11500 12000 12500 13000 13500 14000 14500 15000 15500 16000 16500 17000 17500 18000 18500 19000 19500 20000)

# 创建结果根目录
mkdir -p ./result_data_size

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
    
    # 为该数据集创建目录
    dataset_dir="./result_data_size/${dataset}"
    mkdir -p "$dataset_dir"
    
    echo "Dataset directory: $dataset_dir"
    echo ""
    
    # 测试每个数据大小
    for data_size in "${data_sizes[@]}"; do
        # 构建输入文件路径
        input_file="./exp_data_size/${dataset}_${data_size}.txt"
        
        # 检查输入文件是否存在
        if [ ! -f "$input_file" ]; then
            echo "  Warning: Input file $input_file not found, skipping..."
            continue
        fi
        
        echo "  Testing data size: $data_size"
        
        # 构建输出文件路径
        output_file="${dataset_dir}/${dataset}_${data_size}"
        
        echo "    Input file: $input_file"
        echo "    Output file: $output_file"
        echo "    Running compression with default parameters..."
        
        # 运行 record_compress 并捕获输出以提取压缩时间
        # 使用默认参数：compressor=none, window_size=8, threshold=0.06, block_size=327680000, distance=minhash, use_approx=true, q_value=3
        output_log=$(./record_compress "$input_file" "$output_file" "lzma" 8 0.06 327680000 "minhash" "true" 3 2>&1)
        
        # 检查命令是否成功执行
        if [ $? -eq 0 ]; then
            echo "    ✓ Successfully processed ${dataset} with data size ${data_size}"
            
            # 从输出中提取压缩时间
            compression_time=$(echo "$output_log" | grep "Compressing Total time" | grep -o '[0-9.]*' | head -1)
            if [ -n "$compression_time" ]; then
                echo "    Compression time: ${compression_time} seconds"
            else
                echo "    Warning: Could not extract compression time from output"
            fi
        else
            echo "    ✗ Error processing ${dataset} with data size ${data_size}"
        fi
        
        echo ""
    done
    
    echo "Completed all data sizes for $dataset"
    echo ""
done

echo "=========================================="
echo "All datasets processing completed!"
echo "=========================================="

# 显示结果统计
echo "Generated files by dataset:"
for dataset in "${datasets[@]}"; do
    dataset_dir="./result_data_size/${dataset}"
    if [ -d "$dataset_dir" ]; then
        csv_count=$(ls -1 "$dataset_dir"/*.csv 2>/dev/null | wc -l)
        echo "  $dataset: $csv_count CSV files"
    fi
done

echo ""
echo "Total CSV files generated:"
find ./result_data_size -name "*.csv" | wc -l

echo ""
echo "Directory structure:"
tree ./result_data_size 2>/dev/null || find ./result_data_size -type d | sort

echo ""
echo "Summary statistics:"
echo "Total datasets: ${#datasets[@]}"
echo "Total data sizes per dataset: ${#data_sizes[@]}"
echo "Total experiments: $((${#datasets[@]} * ${#data_sizes[@]}))"

echo ""
echo "All processing completed! Check ./result_data_size/ for compressed files."