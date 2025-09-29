#!/bin/bash

# 脚本用于运行 record_compress 处理 exp_distance 中的文件
# 测试不同距离函数的压缩效果
# 结果按距离函数分类输出到 result_distance/[distance_function]/
# 使用 approx 压缩，参数与 test3_distance_function.cpp 一致

# 定义数据集列表
datasets=("Android" "Apache" "Linux" "Mac" "OpenStack" "Proxifier" "Spark" "SSH" "Thunderbird" "Zookeeper")
# datasets=("Linux" "Mac")

# 定义距离函数列表
distance_functions=("cosine" "minhash" "qgram")
# distance_functions=("minhash")

# 定义阈值参数范围：从0.0到1.0，步长0.02
parameters=()
for p in $(seq 0.0 0.02 1.0); do
    parameters+=($(printf "%.2f" $p))
done

# 创建结果根目录
mkdir -p ./result_distance

# 检查 record_compress 可执行文件是否存在
if [ ! -f "./record_compress" ]; then
    echo "Error: record_compress executable not found!"
    echo "Please make sure record_compress is in the current directory"
    exit 1
fi

# 为每个距离函数创建目录
for distance_func in "${distance_functions[@]}"; do
    mkdir -p "./result_distance/${distance_func}"
done

echo "=========================================="
echo "Starting Distance Function Testing"
echo "=========================================="
echo "Total datasets: ${#datasets[@]}"
echo "Total distance functions: ${#distance_functions[@]}"
echo "Total parameters: ${#parameters[@]}"
echo "Total experiments: $((${#datasets[@]} * ${#distance_functions[@]} * ${#parameters[@]}))"
echo ""

# 测试每个距离函数
for distance_func in "${distance_functions[@]}"; do
    echo "=========================================="
    echo "Testing distance function: $distance_func"
    echo "=========================================="
    
    # 创建该距离函数的结果目录
    distance_dir="./result_distance/${distance_func}"
    
    # 创建时间成本CSV文件
    time_csv="${distance_dir}/time_cost.csv"
    
    # 创建CSV头部
    echo -n "Distance" > "$time_csv"
    for param in "${parameters[@]}"; do
        echo -n ",${param}" >> "$time_csv"
    done
    echo "" >> "$time_csv"
    
    # 处理每个数据集
    for dataset in "${datasets[@]}"; do
        echo "Processing dataset: $dataset"
        
        # 构建输入文件路径
        input_file="./exp_distance/${dataset}_20000.txt"
        
        # 检查输入文件是否存在
        if [ ! -f "$input_file" ]; then
            echo "  Warning: Input file $input_file not found, skipping..."
            # 为该数据集添加一行N/A数据
            echo -n "${dataset}" >> "$time_csv"
            for param in "${parameters[@]}"; do
                echo -n ",N/A" >> "$time_csv"
            done
            echo "" >> "$time_csv"
            continue
        fi
        
        # 为该数据集创建目录
        dataset_dir="${distance_dir}/${dataset}"
        mkdir -p "$dataset_dir"
        
        # 开始该数据集的行
        echo -n "${dataset}" >> "$time_csv"
        
        # 测试每个阈值参数
        for param in "${parameters[@]}"; do
            echo "  Testing threshold: $param"
            
            # 构建输出文件路径
            output_file="${dataset_dir}/${dataset}_${param}"
            
            echo "    Input file: $input_file"
            echo "    Output file: $output_file"
            echo "    Distance function: $distance_func"
            echo "    Threshold: $param"
            echo "    Running compression..."
            
            # 运行 record_compress 并捕获输出以提取压缩时间
            # 使用默认参数：compressor=lzma, window_size=8, block_size=32768000, use_approx=true, q_value=3
            output_log=$(./record_compress "$input_file" "$output_file" "lzma" 8 "$param" 32768000 "$distance_func" "true" 3 2>&1)
            
            # 检查命令是否成功执行
            if [ $? -eq 0 ]; then
                echo "    ✓ Successfully processed ${dataset} with threshold ${param}"
                
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
                echo "    ✗ Error processing ${dataset} with threshold ${param}"
                echo -n ",N/A" >> "$time_csv"
            fi
            
            echo ""
        done
        
        echo "" >> "$time_csv"
        echo "Completed all parameters for $dataset"
        echo ""
    done
    
    echo "✓ Results for $distance_func written to: $time_csv"
    echo ""
done

echo "=========================================="
echo "All distance function testing completed!"
echo "=========================================="

# 显示结果统计
echo "Generated files by distance function:"
for distance_func in "${distance_functions[@]}"; do
    distance_dir="./result_distance/${distance_func}"
    if [ -d "$distance_dir" ]; then
        csv_count=$(ls -1 "$distance_dir"/*.csv 2>/dev/null | wc -l)
        echo "  $distance_func: $csv_count CSV files"
    fi
done

echo ""
echo "Total CSV files generated:"
find ./result_distance -name "*.csv" | wc -l

echo ""
echo "Directory structure:"
tree ./result_distance 2>/dev/null || find ./result_distance -type d | sort

echo ""
echo "Summary statistics:"
echo "Total datasets: ${#datasets[@]}"
echo "Total distance functions: ${#distance_functions[@]}"
echo "Total parameters: ${#parameters[@]}"
echo "Total experiments: $((${#datasets[@]} * ${#distance_functions[@]} * ${#parameters[@]}))"

echo ""
echo "All processing completed! Check ./result_distance/ for results."
