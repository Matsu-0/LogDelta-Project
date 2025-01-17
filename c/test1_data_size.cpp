#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <chrono>
#include "record_compress.hpp"
#include "utils.hpp"

// Define dataset names
const std::vector<std::string> datasets = {"Android", "Apache", "HPC", "Mac", "OpenStack", "Spark", "Zookeeper", "SSH", "Linux", "Proxifier", "Thunderbird"};

void approx_encoding() {
    // Define parameter range
    std::vector<int> parameters;
    for (int p = 1000; p < 20500; p += 500) {
        parameters.push_back(p);
    }

    std::string input_path = "../datasets/test1_data_size/";
    std::string output_path = "../result/result_approx/test1_data_size/";

    // Ensure output directory exists
    if (!ensure_directory_exists(output_path)) {
        std::cerr << "Failed to create output directory: " << output_path << std::endl;
        return;
    }

    // Store time results for each dataset
    std::map<std::string, std::vector<double>> time_sets;

    // Calculate total tasks for progress display
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
            
            double time = main_encoding_compress(input_file_name, output_file_name, 
                                                 DefaultParams::WINDOW_SIZE,
                                                 DefaultParams::LOG_LENGTH,
                                                 DefaultParams::THRESHOLD,
                                                 p,
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
    first_column.push_back("Parameter");  // Header
    for (int p : parameters) {
        first_column.push_back(std::to_string(p));
    }

    if (write_csv(csv_path, time_sets, datasets, first_column)) {
        std::cout << "\nAll tasks completed. Results written to: " << csv_path << std::endl;
    } else {
        std::cerr << "\nFailed to write results to: " << csv_path << std::endl;
    }
}

void exact_encoding() {
    // Define parameter range
    std::vector<int> parameters;
    for (int p = 1000; p < 20500; p += 500) {
        parameters.push_back(p);
    }

    std::string input_path = "../datasets/test1_data_size/";
    std::string output_path = "../result/result_exact/test1_data_size/";

    // Ensure output directory exists
    if (!ensure_directory_exists(output_path)) {
        std::cerr << "Failed to create output directory: " << output_path << std::endl;
        return;
    }

    // Store time results for each dataset
    std::map<std::string, std::vector<double>> time_sets;

    // Calculate total tasks for progress display
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
            
            double time = main_encoding_compress(input_file_name, output_file_name, 
                                               DefaultParams::WINDOW_SIZE,
                                               DefaultParams::LOG_LENGTH,
                                               DefaultParams::THRESHOLD,
                                               p,
                                               DefaultParams::COMPRESSOR,
                                               DefaultParams::DISTANCE,
                                               false);  // Set use_approx to false
            time_list.push_back(time);
            
            std::cout << "Time cost: " << time << " seconds" << std::endl;
        }
        time_sets[d] = time_list;
    }

    // Write results to CSV file
    std::string csv_path = output_path + "time_cost.csv";
    std::vector<std::string> first_column;
    first_column.push_back("Parameter");  // Header
    for (int p : parameters) {
        first_column.push_back(std::to_string(p));
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
        exact_encoding();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}