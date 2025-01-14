#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <filesystem>
#include <chrono>
#include "record_compress.hpp"

namespace fs = std::filesystem;

// 定义数据集名称
const std::vector<std::string> datasets = {"Apache", "Linux", "Proxifier", "Thunderbird"};

// 创建目录的辅助函数
void mkdir(const std::string& path) {
    if (!fs::exists(path)) {
        fs::create_directories(path);
    }
}

// 写入CSV文件的辅助函数
void write_csv(const std::string& filepath, 
              const std::map<std::string, std::vector<double>>& time_sets,
              const std::vector<int>& parameters) {
    std::ofstream file(filepath);
    
    // 写入表头
    for (size_t i = 0; i < datasets.size(); ++i) {
        file << datasets[i];
        if (i < datasets.size() - 1) file << ",";
    }
    file << "\n";
    
    // 写入数据
    for (size_t i = 0; i < parameters.size(); ++i) {
        for (size_t j = 0; j < datasets.size(); ++j) {
            file << time_sets.at(datasets[j])[i];
            if (j < datasets.size() - 1) file << ",";
        }
        file << "\n";
    }
}

// void exact_encoding() {
//     // 定义参数范围
//     std::vector<int> parameters;
//     for (int p = 1000; p < 8000; p += 500) {
//         parameters.push_back(p);
//     }

//     std::string input_path = "./datasets/test1_data_size/";
//     std::string output_path = "./result/result_exact/test1_data_size/";

//     // 存储每个数据集的时间结果
//     std::map<std::string, std::vector<double>> time_sets;

//     for (const auto& d : datasets) {
//         std::vector<double> time_list;
//         std::string tmp_path = output_path + d;
//         mkdir(tmp_path);

//         for (int p : parameters) {
//             std::string input_file_name = input_path + d + "_" + std::to_string(p) + ".txt";
//             std::string output_file_name = output_path + d + "/" + d + "_" + std::to_string(p);
            
//             double time = main_encoding_compress(input_file_name, output_file_name);
//             time_list.push_back(time);
//         }
//         time_sets[d] = time_list;
//     }

//     // 写入结果到CSV文件
//     write_csv(output_path + "time_cost.csv", time_sets, parameters);
// }

void approx_encoding() {
    // 定义参数范围
    std::vector<int> parameters;
    for (int p = 1000; p < 20500; p += 500) {
        parameters.push_back(p);
    }

    std::string input_path = "./datasets/test1_data_size/";
    std::string output_path = "./result/result_approx/test1_data_size/";

    // 存储每个数据集的时间结果
    std::map<std::string, std::vector<double>> time_sets;

    for (const auto& d : datasets) {
        std::vector<double> time_list;
        std::string tmp_path = output_path + d;
        mkdir(tmp_path);

        for (int p : parameters) {
            std::string input_file_name = input_path + d + "_" + std::to_string(p) + ".txt";
            std::string output_file_name = output_path + d + "/" + d + "_" + std::to_string(p);
            
            double time = main_encoding_compress_approx(input_file_name, output_file_name);
            time_list.push_back(time);
        }
        time_sets[d] = time_list;
    }

    // 写入结果到CSV文件
    write_csv(output_path + "time_cost.csv", time_sets, parameters);
}

int main() {
    try {
        // exact_encoding();
        approx_encoding();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}