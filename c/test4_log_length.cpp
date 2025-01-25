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

void cleanup_resources(std::map<std::string, std::vector<double>>& time_sets) {
    // Clean vectors in map
    for (auto& pair : time_sets) {
        std::vector<double>().swap(pair.second);
    }
    // Clean the map itself
    std::map<std::string, std::vector<double>>().swap(time_sets);
}

void approx_encoding() {
    // Define parameter range (powers of 2: 16, 32, 64, 128, 256, 512, 1024)
    std::vector<int> parameters;
    for (int p = 16; p <= 1024; p *= 2) {
        parameters.push_back(p);
    }

    std::string input_path = "../datasets/test_dataset/";
    std::string output_path = "../result/result_approx/test4_log_length/";

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
                      << "Dataset: " << d << ", Log length: " << p << std::endl;

            std::string input_file_name = input_path + d + ".log";
            std::string output_file_name = output_path + d + "/" + d + "_" + std::to_string(p);
            
            if (!std::ifstream(input_file_name)) {
                std::cerr << "Input file does not exist: " << input_file_name << std::endl;
                continue;
            }
            
            // Use fixed window_size=8, threshold=0.06, block_size=32768, only change log_length
            double time = main_encoding_compress(input_file_name, output_file_name,
                                               DefaultParams::WINDOW_SIZE,
                                               p,
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
    first_column.push_back("LogLength");  // Header
    for (int p : parameters) {
        first_column.push_back(std::to_string(p));
    }

    bool write_success = write_csv(csv_path, time_sets, datasets, first_column);
    
    // Clean up resources before exit
    cleanup_resources(time_sets);

    if (write_success) {
        std::cout << "\nAll tasks completed. Results written to: " << csv_path << std::endl;
    } else {
        std::cerr << "\nFailed to write results to: " << csv_path << std::endl;
    }
}

void exact_encoding() {
    // Define parameter range (powers of 2: 16, 32, 64, 128, 256, 512, 1024)
    std::vector<int> parameters;
    for (int p = 16; p <= 1024; p *= 2) {
        parameters.push_back(p);
    }

    std::string input_path = "../datasets/test_dataset/";
    std::string output_path = "../result/result_exact/test4_log_length/";

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
                      << "Dataset: " << d << ", Log length: " << p << std::endl;

            std::string input_file_name = input_path + d + ".log";
            std::string output_file_name = output_path + d + "/" + d + "_" + std::to_string(p);
            
            if (!std::ifstream(input_file_name)) {
                std::cerr << "Input file does not exist: " << input_file_name << std::endl;
                continue;
            }
            
            double time = main_encoding_compress(input_file_name, output_file_name,
                                               DefaultParams::WINDOW_SIZE,
                                               p,
                                               DefaultParams::THRESHOLD,
                                               DefaultParams::BLOCK_SIZE,
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
    first_column.push_back("LogLength");  // Header
    for (int p : parameters) {
        first_column.push_back(std::to_string(p));
    }

    bool write_success = write_csv(csv_path, time_sets, datasets, first_column);
    
    // Clean up resources before exit
    cleanup_resources(time_sets);

    if (write_success) {
        std::cout << "\nAll tasks completed. Results written to: " << csv_path << std::endl;
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
        
        std::cout << "\n=== All Tasks Completed ===\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
