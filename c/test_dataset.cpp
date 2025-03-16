#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <chrono>
#include "record_compress.hpp"
#include "utils.hpp"

// Define dataset parameters (threshold, log_length)
const std::map<std::string, std::pair<double, int>> dataset_params = {
    {"Zookeeper",    {0.28, 32}},
    {"OpenStack",    {0.06, 256}},
    {"Spark",        {0.16, 32}},
    {"Linux",        {0.06, 256}},
    {"Mac",          {0.12, 32}},
    {"Thunderbird",  {0.16, 256}},
    {"Apache",       {0.10, 256}},
    {"SSH",          {0.20, 32}},
    {"Proxifier",    {0.02, 256}},
    {"Android",      {0.06, 32}}
};

void test_datasets() {
    std::string input_path = "../datasets/test_dataset/";
    std::string output_path = "../result/result_approx/test_dataset/";

    // Ensure output directory exists
    if (!ensure_directory_exists(output_path)) {
        std::cerr << "Failed to create output directory: " << output_path << std::endl;
        return;
    }

    // Process each dataset
    for (const auto& [dataset, params] : dataset_params) {
        std::cout << "\n=== Processing dataset: " << dataset << " ===" << std::endl;
        std::cout << "Parameters - threshold: " << params.first 
                  << ", log_length: " << params.second << std::endl;

        std::string input_file = input_path + dataset + ".log";
        std::string output_file = output_path + dataset;
        
        if (!std::ifstream(input_file)) {
            std::cerr << "Input file does not exist: " << input_file << std::endl;
            continue;
        }
        
        // Use custom threshold and log_length, default for other parameters
        double time = main_encoding_compress(
            input_file,
            output_file,
            DefaultParams::WINDOW_SIZE,
            params.second,           // Custom log_length
            params.first,           // Custom threshold
            DefaultParams::BLOCK_SIZE,
            DefaultParams::COMPRESSOR,
            DefaultParams::DISTANCE,
            true  // Use approximate matching
        );
        
        std::cout << "Time cost: " << time << " seconds" << std::endl;
    }
}

int main() {
    try {
        std::cout << "\n=== Starting Dataset Compression Tests ===\n" << std::endl;
        test_datasets();
        std::cout << "\n=== All Tests Completed ===\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
