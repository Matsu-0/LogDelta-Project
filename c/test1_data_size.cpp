#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <sys/stat.h>
#include <chrono>
#include "record_compress.hpp"
#include "utils.hpp"

// 定义数据集名称
const std::vector<std::string> datasets = {"Apache", "Linux", "Proxifier", "Thunderbird"};

// 创建目录的辅助函数，返回是否成功
bool create_directory(const std::string& path) {
    #ifdef _WIN32
        return _mkdir(path.c_str()) == 0;
    #else
        return mkdir(path.c_str(), 0777) == 0;
    #endif
}

// 检查目录是否存在
bool directory_exists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}

// 确保目录存在，如果需要则创建完整的目录路径
bool ensure_directory_exists(const std::string& path) {
    size_t pos = 0;
    std::string dir;
    while ((pos = path.find('/', pos)) != std::string::npos) {
        dir = path.substr(0, pos++);
        if (dir.empty()) continue;
        if (!directory_exists(dir)) {
            if (!create_directory(dir)) {
                std::cerr << "Failed to create directory: " << dir << std::endl;
                return false;
            }
        }
    }
    if (!directory_exists(path)) {
        return create_directory(path);
    }
    return true;
}

// 写入CSV文件的辅助函数
bool write_csv(const std::string& filepath, 
              const std::map<std::string, std::vector<double>>& time_sets,
              const std::vector<int>& parameters) {
    // 确保目录存在
    std::string dir = filepath.substr(0, filepath.find_last_of('/'));
    if (!ensure_directory_exists(dir)) {
        std::cerr << "Failed to create directory for CSV file: " << dir << std::endl;
        return false;
    }

    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filepath << std::endl;
        return false;
    }
    
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

    file.close();
    return true;
}

void approx_encoding() {
    // 定义参数范围
    std::vector<int> parameters;
    for (int p = 1000; p < 20500; p += 500) {
        parameters.push_back(p);
    }

    std::string input_path = "../datasets/test1_data_size/";
    std::string output_path = "../result/result_approx/test1_data_size/";

    // 确保输出目录存在
    if (!ensure_directory_exists(output_path)) {
        std::cerr << "Failed to create output directory: " << output_path << std::endl;
        return;
    }

    // 存储每个数据集的时间结果
    std::map<std::string, std::vector<double>> time_sets;

    // 计算总任务数用于显示进度
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

        for (int p : parameters) {
            current_task++;
            std::cout << "Progress: [" << current_task << "/" << total_tasks << "] "
                      << "Dataset: " << d << ", Parameter: " << p << std::endl;

            std::string input_file_name = input_path + d + "_" + std::to_string(p) + ".txt";
            std::string output_file_name = output_path + d + "/" + d + "_" + std::to_string(p);
            
            if (!std::ifstream(input_file_name)) {
                std::cerr << "Input file does not exist: " << input_file_name << std::endl;
                continue;
            }
            
            double time = main_encoding_compress_approx(input_file_name, output_file_name);
            time_list.push_back(time);
            
            std::cout << "Time cost: " << time << " seconds" << std::endl;
        }
        time_sets[d] = time_list;
    }

    // 写入结果到CSV文件
    std::string csv_path = output_path + "time_cost.csv";
    if (write_csv(csv_path, time_sets, parameters)) {
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