#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <chrono>
#include "record_compress.hpp"
#include "utils.hpp"
#include "distance.hpp"

// 定义数据集名称
const std::vector<std::string> datasets = {"Android", "Apache", "HPC", "Mac", "OpenStack", "Spark", "Zookeeper", "SSH", "Linux", "Proxifier", "Thunderbird"};

// 添加距离函数类型的定义
const std::vector<std::pair<std::string, DistanceType>> distance_functions = {
    {"COSINE", DistanceType::COSINE},
    {"MINHASH", DistanceType::MINHASH},
    {"QGRAM", DistanceType::QGRAM}
};

void approx_encoding() {
    // 定义参数范围 (0.0 到 1.0，步长0.02)
    std::vector<double> parameters;
    for (double p = 0.0; p <= 1.0; p += 0.02) {
        parameters.push_back(p);
    }

    // 为每种距离函数创建结果目录
    for (const auto& [dist_name, _] : distance_functions) {
        std::string output_path = "../result/result_approx/test3_distance/" + dist_name + "/";
        if (!ensure_directory_exists(output_path)) {
            std::cerr << "Failed to create output directory: " << output_path << std::endl;
            return;
        }
    }

    // 对每种距离函数进行测试
    for (const auto& [dist_name, dist_type] : distance_functions) {
        std::cout << "\n=== Testing distance function: " << dist_name << " ===" << std::endl;
        
        std::string output_path = "../result/result_approx/test3_distance/" + dist_name + "/";
        std::map<std::string, std::vector<double>> time_sets;

        // 计算总任务数
        size_t total_tasks = datasets.size() * parameters.size();
        size_t current_task = 0;

        for (const auto& d : datasets) {
            std::vector<double> time_list;
            std::string tmp_path = output_path + d;
            
            if (!ensure_directory_exists(tmp_path)) {
                std::cerr << "Failed to create directory: " << tmp_path << std::endl;
                continue;
            }

            std::cout << "\n=== Processing dataset: " << d << " ===" << std::endl;

            for (double p : parameters) {
                current_task++;
                std::cout << "Progress: [" << current_task << "/" << total_tasks << "] "
                          << "Dataset: " << d << ", Distance threshold: " << p 
                          << ", Function: " << dist_name << std::endl;

                std::string input_file_name = "../datasets/test_dataset/" + d + ".log";
                std::string output_file_name = output_path + d + "/" + d + "_" + std::to_string(p);
                
                if (!std::ifstream(input_file_name)) {
                    std::cerr << "Input file does not exist: " << input_file_name << std::endl;
                    continue;
                }
                
                double time = main_encoding_compress_approx(input_file_name, output_file_name, 
                                                          8, 256, p, 32768, 
                                                          CompressorType::NONE, dist_type);
                time_list.push_back(time);
                
                std::cout << "Time cost: " << time << " seconds" << std::endl;
            }
            time_sets[d] = time_list;
        }

        // 写入结果到CSV文件
        std::string csv_path = output_path + "time_cost.csv";
        std::vector<std::string> first_column;
        first_column.push_back("Distance");
        for (double p : parameters) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%.2f", p);
            first_column.push_back(buffer);
        }

        if (write_csv(csv_path, time_sets, datasets, first_column)) {
            std::cout << "\nResults for " << dist_name << " written to: " << csv_path << std::endl;
        } else {
            std::cerr << "\nFailed to write results for " << dist_name << " to: " << csv_path << std::endl;
        }
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
