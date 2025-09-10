#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <chrono>
#include "record_compress.hpp"
#include "utils.hpp"
#include "distance.hpp"

// Define dataset names
const std::vector<std::string> datasets = {"Android", "Apache", "Mac", "OpenStack", "Spark", "Zookeeper", "SSH", "Linux", "Proxifier", "Thunderbird"};
// const std::vector<std::string> datasets = {"Apache"};

void cleanup_resources(std::map<std::string, std::vector<double>>& time_sets) {
    for (auto& pair : time_sets) {
        std::vector<double>().swap(pair.second);
    }
    std::map<std::string, std::vector<double>>().swap(time_sets);
}

void approx_encoding() {
    // 只测试2, 4, 8, 16, 32, 64, 128
    std::vector<int> window_sizes = {2, 4, 8, 16, 32, 64, 128};

    std::string output_path = "../result_new/result_approx/test5_window_size/";
    if (!ensure_directory_exists(output_path)) {
        std::cerr << "Failed to create output directory: " << output_path << std::endl;
        return;
    }

    size_t total_tasks = datasets.size() * window_sizes.size();
    size_t current_task = 0;
    std::map<std::string, std::vector<double>> time_sets;

    for (const auto& d : datasets) {
        std::vector<double> time_list;
        std::string tmp_path = output_path + d;
        if (!ensure_directory_exists(tmp_path)) {
            std::cerr << "Failed to create directory: " << tmp_path << std::endl;
            continue;
        }
        std::cout << "\n=== Processing dataset: " << d << " ===" << std::endl;
        for (int w : window_sizes) {
            current_task++;
            std::cout << "Progress: [" << current_task << "/" << total_tasks << "] "
                      << "Dataset: " << d << ", Window size: " << w << std::endl;
            std::string input_file_name = "../../datasets/test1_data_size/" + d + "_20000.txt";
            std::string output_file_name = output_path + d + "/" + d + "_" + std::to_string(w);
            if (!std::ifstream(input_file_name)) {
                std::cerr << "Input file does not exist: " << input_file_name << std::endl;
                continue;
            }
            double time = main_encoding_compress(input_file_name, output_file_name,
                                               w,
                                               DefaultParams::THRESHOLD,
                                               DefaultParams::BLOCK_SIZE,
                                               DefaultParams::COMPRESSOR,
                                               DefaultParams::DISTANCE,
                                               DefaultParams::USE_APPROX);
            time_list.push_back(time);
            std::cout << "Time cost: " << time << " seconds" << std::endl;
        }
        time_sets[d] = time_list;
    }
    // Write results to CSV file
    std::string csv_path = output_path + "time_cost.csv";
    std::vector<std::string> first_column;
    for (int w : window_sizes) {
        first_column.push_back(std::to_string(w));
    }
    bool write_success = write_csv(csv_path, time_sets, datasets, first_column);
    cleanup_resources(time_sets);
    if (write_success) {
        std::cout << "\nResults written to: " << csv_path << std::endl;
    } else {
        std::cerr << "\nFailed to write results to: " << csv_path << std::endl;
    }
}

void exact_encoding() {
    // 只测试2, 4, 8, 16, 32, 64, 128
    std::vector<int> window_sizes = {2, 4, 8, 16, 32, 64, 128};

    std::string output_path = "../result_new/result_exact/test5_window_size/";
    if (!ensure_directory_exists(output_path)) {
        std::cerr << "Failed to create output directory: " << output_path << std::endl;
        return;
    }

    size_t total_tasks = datasets.size() * window_sizes.size();
    size_t current_task = 0;
    std::map<std::string, std::vector<double>> time_sets;

    for (const auto& d : datasets) {
        std::vector<double> time_list;
        std::string tmp_path = output_path + d;
        if (!ensure_directory_exists(tmp_path)) {
            std::cerr << "Failed to create directory: " << tmp_path << std::endl;
            continue;
        }
        std::cout << "\n=== Processing dataset: " << d << " ===" << std::endl;
        for (int w : window_sizes) {
            current_task++;
            std::cout << "Progress: [" << current_task << "/" << total_tasks << "] "
                      << "Dataset: " << d << ", Window size: " << w << std::endl;
            std::string input_file_name = "../../datasets/test1_data_size/" + d + "_20000.txt";
            std::string output_file_name = output_path + d + "/" + d + "_" + std::to_string(w);
            if (!std::ifstream(input_file_name)) {
                std::cerr << "Input file does not exist: " << input_file_name << std::endl;
                continue;
            }
            double time = main_encoding_compress(input_file_name, output_file_name,
                                               w,
                                               DefaultParams::THRESHOLD,
                                               DefaultParams::BLOCK_SIZE,
                                               DefaultParams::COMPRESSOR,
                                               DefaultParams::DISTANCE,
                                               false); // use_approx = false
            time_list.push_back(time);
            std::cout << "Time cost: " << time << " seconds" << std::endl;
        }
        time_sets[d] = time_list;
    }
    // Write results to CSV file
    std::string csv_path = output_path + "time_cost.csv";
    std::vector<std::string> first_column;
    for (int w : window_sizes) {
        first_column.push_back(std::to_string(w));
    }
    bool write_success = write_csv(csv_path, time_sets, datasets, first_column);
    cleanup_resources(time_sets);
    if (write_success) {
        std::cout << "\nResults written to: " << csv_path << std::endl;
    } else {
        std::cerr << "\nFailed to write results to: " << csv_path << std::endl;
    }
}

int main() {
    try {
        std::cout << "\n=== Starting Approximate Encoding ===\n" << std::endl;
        approx_encoding();
        std::cout << "\n=== Approximate Encoding Completed ===\n" << std::endl;

        std::cout << "\n=== Starting Exact Encoding ===\n" << std::endl;
        exact_encoding();
        std::cout << "\n=== Exact Encoding Completed ===\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
