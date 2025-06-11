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
// const std::vector<std::string> datasets = {"HPC", "Mac", "OpenStack"};

void cleanup_resources(std::map<std::string, std::vector<double>>& time_sets) {
    // Clean vectors in map
    for (auto& pair : time_sets) {
        std::vector<double>().swap(pair.second);
    }
    // Clean the map itself
    std::map<std::string, std::vector<double>>().swap(time_sets);
}

void approx_encoding() {
    std::string input_path = "../datasets/test_dataset/";
    std::string output_path = "../result_new/result_approx/test_dataset/";
    std::map<std::string, std::vector<double>> time_sets;

    // Ensure output directory exists
    if (!ensure_directory_exists(output_path)) {
        std::cerr << "Failed to create output directory: " << output_path << std::endl;
        return;
    }

    // Process each dataset
    for (const auto& d : datasets) {
        std::vector<double> time_list;  // Create time_list for each dataset
        std::cout << "\n=== Processing dataset: " << d << " ===" << std::endl;

        std::string input_file_name = input_path + d + ".log";
        std::string output_file_name = output_path + d;  // Direct in test_dataset directory
        
        if (!std::ifstream(input_file_name)) {
            std::cerr << "Input file does not exist: " << input_file_name << std::endl;
            continue;
        }
        
        double time = main_encoding_compress(input_file_name, output_file_name, 
                                           DefaultParams::WINDOW_SIZE,
                                           DefaultParams::LOG_LENGTH,
                                           DefaultParams::THRESHOLD,    
                                           DefaultParams::BLOCK_SIZE,
                                           DefaultParams::COMPRESSOR,
                                           DefaultParams::DISTANCE,
                                           DefaultParams::USE_APPROX);
        
        std::cout << "Time cost: " << time << " seconds" << std::endl;
        time_list.push_back(time);  // Add time to list
        time_sets[d] = time_list;   // Store list in map
        
        // 添加调试信息
        std::cout << "Dataset " << d << " time_list size: " << time_list.size() << std::endl;
        std::cout << "time_list values: ";
        for (const auto& t : time_list) {
            std::cout << t << " ";
        }
        std::cout << std::endl;
    }

    // Write results to CSV file
    std::string csv_path = output_path + "time_cost.csv";
    std::vector<std::string> first_column = {"Time"};

    std::cout << "Preparing to write CSV..." << std::endl;
    std::cout << "Number of datasets: " << datasets.size() << std::endl;
    std::cout << "Size of time_sets: " << time_sets.size() << std::endl;
    
    // 添加更多调试信息
    for (const auto& pair : time_sets) {
        std::cout << "Dataset: " << pair.first << ", Values: ";
        for (const auto& t : pair.second) {
            std::cout << t << " ";
        }
        std::cout << std::endl;
    }

    bool write_success = write_csv(csv_path, time_sets, datasets, first_column);
    
    std::cout << "CSV write completed with status: " << (write_success ? "success" : "failure") << std::endl;

    cleanup_resources(time_sets);
}

void exact_encoding() {
    std::string input_path = "../datasets/test_dataset/";
    std::string output_path = "../result_new/result_exact/test_dataset/";
    std::map<std::string, std::vector<double>> time_sets;

    // Ensure output directory exists
    if (!ensure_directory_exists(output_path)) {
        std::cerr << "Failed to create output directory: " << output_path << std::endl;
        return;
    }

    // Process each dataset
    for (const auto& d : datasets) {
        std::vector<double> time_list;
        std::cout << "\n=== Processing dataset: " << d << " ===" << std::endl;

        std::string input_file_name = input_path + d + ".log";
        std::string output_file_name = output_path + d;  // Direct in test_dataset directory
        
        if (!std::ifstream(input_file_name)) {
            std::cerr << "Input file does not exist: " << input_file_name << std::endl;
            continue;
        }
        
        double time = main_encoding_compress(input_file_name, output_file_name, 
                                           DefaultParams::WINDOW_SIZE,
                                           DefaultParams::LOG_LENGTH,
                                           DefaultParams::THRESHOLD,
                                           DefaultParams::BLOCK_SIZE,
                                           DefaultParams::COMPRESSOR,
                                           DefaultParams::DISTANCE,
                                           false);  // Set use_approx to false
        
        std::cout << "Time cost: " << time << " seconds" << std::endl;
        time_list.push_back(time);  // Add time to list
        time_sets[d] = time_list;   // Store list in map
    }

    // Write results to CSV file
    std::string csv_path = output_path + "time_cost.csv";
    std::vector<std::string> first_column = {"Time"};

    std::cout << "Preparing to write CSV..." << std::endl;
    std::cout << "Number of datasets: " << datasets.size() << std::endl;
    std::cout << "Size of time_sets: " << time_sets.size() << std::endl;

    bool write_success = write_csv(csv_path, time_sets, datasets, first_column);
    
    std::cout << "CSV write completed with status: " << (write_success ? "success" : "failure") << std::endl;

    cleanup_resources(time_sets);
}

int main() {
    try {
        std::cout << "\n=== Starting Approximate Encoding ===\n" << std::endl;
        approx_encoding();
        std::cout << "\n=== Approximate Encoding Completed ===\n" << std::endl;
        
        // Add a small delay to ensure resources are properly released
        std::cout.flush();
        
        // std::cout << "\n=== Starting Exact Encoding ===\n" << std::endl;
        // exact_encoding();
        
        // std::cout << "\n=== All Tasks Completed ===\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
}
