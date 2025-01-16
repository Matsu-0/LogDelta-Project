#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <chrono>
#include "record_compress.hpp"
#include "utils.hpp"

// 定义数据集名称和压缩器类型
const std::vector<std::string> datasets = {"Android", "Apache", "HPC", "Mac", "OpenStack", "Spark", "Zookeeper", "SSH", "Linux", "Proxifier", "Thunderbird"};
const std::vector<std::pair<std::string, CompressorType>> compressors = {
    {"NONE", CompressorType::NONE},
    {"LZMA", CompressorType::LZMA},
    {"GZIP", CompressorType::GZIP},
    {"ZSTD", CompressorType::ZSTD}
};

void approx_encoding() {
    std::string input_path = "../datasets/test_dataset/";
    std::string output_path = "../result/result_approx/test2_compressor/";

    // 确保输出目录存在
    if (!ensure_directory_exists(output_path)) {
        std::cerr << "Failed to create output directory: " << output_path << std::endl;
        return;
    }

    // 存储每个数据集的时间结果
    std::map<std::string, std::vector<double>> time_sets;

    // 计算总任务数用于显示进度
    size_t total_tasks = datasets.size() * compressors.size();
    size_t current_task = 0;

    for (const auto& d : datasets) {
        std::vector<double> time_list;
        std::string tmp_path = output_path + d;
        
        if (!ensure_directory_exists(tmp_path)) {
            std::cerr << "Failed to create directory: " << tmp_path << std::endl;
            continue;
        }

        std::cout << "\n=== Processing dataset: " << d << " ===" << std::endl;

        for (const auto& [comp_name, comp_type] : compressors) {
            current_task++;
            std::cout << "Progress: [" << current_task << "/" << total_tasks << "] "
                      << "Dataset: " << d << ", Compressor: " << comp_name << std::endl;

            std::string input_file_name = input_path + d + ".log";
            std::string output_file_name = output_path + d + "/" + d + "_" + comp_name;
            
            if (!std::ifstream(input_file_name)) {
                std::cerr << "Input file does not exist: " << input_file_name << std::endl;
                continue;
            }
            
            double time = main_encoding_compress(input_file_name, output_file_name,
                                                DefaultParams::WINDOW_SIZE,
                                                DefaultParams::LOG_LENGTH,
                                                DefaultParams::THRESHOLD,
                                                DefaultParams::BLOCK_SIZE,
                                                comp_type,
                                                DefaultParams::DISTANCE,
                                                DefaultParams::USE_APPROX);
            time_list.push_back(time);
            
            std::cout << "Time cost: " << time << " seconds" << std::endl;
        }
        time_sets[d] = time_list;
    }

    // 写入结果到CSV文件
    std::string csv_path = output_path + "time_cost.csv";
    std::vector<std::string> first_column;
    first_column.push_back("Compressor");  // 表头
    for (const auto& [name, _] : compressors) {
        first_column.push_back(name);
    }
    
    if (write_csv(csv_path, time_sets, datasets, first_column)) {
        std::cout << "\nAll tasks completed. Results written to: " << csv_path << std::endl;
    } else {
        std::cerr << "\nFailed to write results to: " << csv_path << std::endl;
    }
}

int main() {
    try {
        approx_encoding();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
