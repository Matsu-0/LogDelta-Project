#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <chrono>
#include "record_compress.hpp"
#include "utils.hpp"

// Define dataset names and compressor types
const std::vector<std::string> datasets = {"Android", "Apache", "HPC", "Mac", "OpenStack", "Spark", "Zookeeper", "SSH", "Linux", "Proxifier", "Thunderbird"};
const std::vector<std::pair<std::string, CompressorType>> compressors = {
    {"LZMA", CompressorType::LZMA},
    {"GZIP", CompressorType::GZIP},
    {"ZSTD", CompressorType::ZSTD}
};

void cleanup_resources(std::map<std::string, std::vector<double>>& time_sets) {
    // Clean vectors in map
    for (auto& pair : time_sets) {
        std::vector<double>().swap(pair.second);
    }
    // Clean the map itself
    std::map<std::string, std::vector<double>>().swap(time_sets);
}

bool compress_file(const std::string& input_file, const std::string& output_file, CompressorType comp_type) {
    // Read input file into BitBuffer
    BitBuffer buffer;
    if (!buffer.read(input_file)) {
        std::cerr << "Failed to read input file: " << input_file << std::endl;
        return false;
    }
    
    // Write compressed output using BitBuffer
    if (!buffer.write(output_file, "wb", comp_type)) {
        std::cerr << "Failed to write compressed file: " << output_file << std::endl;
        return false;
    }
    
    return true;
}

void approx_encoding() {
    std::string input_path = "../datasets/test_dataset/";
    std::string output_path = "../result/result_approx/test2_compressor/";

    // First phase: Process with NONE compressor
    std::cout << "\n=== Phase 1: Processing with NONE compressor ===\n" << std::endl;
    
    // Ensure output directory exists
    if (!ensure_directory_exists(output_path)) {
        std::cerr << "Failed to create output directory: " << output_path << std::endl;
        return;
    }

    // Process each dataset with NONE compressor
    for (const auto& d : datasets) {
        std::string tmp_path = output_path + d;
        if (!ensure_directory_exists(tmp_path)) {
            std::cerr << "Failed to create directory: " << tmp_path << std::endl;
            continue;
        }

        std::cout << "\n=== Processing dataset: " << d << " with NONE compressor ===" << std::endl;

        std::string input_file_name = input_path + d + ".log";
        std::string output_file_name = output_path + d + "/" + d + "_NONE";
        
        if (!std::ifstream(input_file_name)) {
            std::cerr << "Input file does not exist: " << input_file_name << std::endl;
            continue;
        }
        
        double time = main_encoding_compress(input_file_name, output_file_name,
                                           DefaultParams::WINDOW_SIZE,
                                           DefaultParams::LOG_LENGTH,
                                           DefaultParams::THRESHOLD,
                                           DefaultParams::BLOCK_SIZE,
                                           CompressorType::NONE,
                                           DefaultParams::DISTANCE,
                                           true);  // Use approx
        
        std::cout << "Time cost: " << time << " seconds" << std::endl;
    }

    // Second phase: Process NONE results with other compressors
    std::cout << "\n=== Phase 2: Processing with other compressors ===\n" << std::endl;
    
    std::map<std::string, std::vector<double>> time_sets;

    for (const auto& d : datasets) {
        std::vector<double> time_list;
        std::string none_file = output_path + d + "/" + d + "_NONE";
        
        for (const auto& [comp_name, comp_type] : compressors) {
            std::cout << "\n=== Processing " << d << " with " << comp_name << " ===" << std::endl;
            
            std::string output_file = output_path + d + "/" + d + "_" + comp_name;
            
            if (!std::ifstream(none_file)) {
                std::cerr << "NONE compressed file not found: " << none_file << std::endl;
                continue;
            }
            
            auto start = std::chrono::high_resolution_clock::now();
            bool success = compress_file(none_file, output_file, comp_type);
            auto end = std::chrono::high_resolution_clock::now();
            double time = std::chrono::duration<double>(end - start).count();
            
            if (success) {
                time_list.push_back(time);
                std::cout << "Time cost: " << time << " seconds" << std::endl;
            } else {
                std::cerr << "Compression failed for " << comp_name << std::endl;
                time_list.push_back(-1);
            }
        }
        time_sets[d] = time_list;
    }

    // Write compression times to CSV
    std::string csv_path = output_path + "compression_time.csv";
    std::vector<std::string> first_column;
    first_column.push_back("Compressor");
    for (const auto& [name, _] : compressors) {
        first_column.push_back(name);
    }
    
    bool write_success = write_csv(csv_path, time_sets, datasets, first_column);
    cleanup_resources(time_sets);

    if (write_success) {
        std::cout << "\nAll tasks completed. Results written to: " << csv_path << std::endl;
    } else {
        std::cerr << "\nFailed to write results to: " << csv_path << std::endl;
    }
}

void exact_encoding() {
    std::string input_path = "../datasets/test_dataset/";
    std::string output_path = "../result/result_exact/test2_compressor/";

    // First phase: Process with NONE compressor
    std::cout << "\n=== Phase 1: Processing with NONE compressor ===\n" << std::endl;
    
    // Ensure output directory exists
    if (!ensure_directory_exists(output_path)) {
        std::cerr << "Failed to create output directory: " << output_path << std::endl;
        return;
    }

    // Process each dataset with NONE compressor
    for (const auto& d : datasets) {
        std::string tmp_path = output_path + d;
        if (!ensure_directory_exists(tmp_path)) {
            std::cerr << "Failed to create directory: " << tmp_path << std::endl;
            continue;
        }

        std::cout << "\n=== Processing dataset: " << d << " with NONE compressor ===" << std::endl;

        std::string input_file_name = input_path + d + ".log";
        std::string output_file_name = output_path + d + "/" + d + "_NONE";
        
        if (!std::ifstream(input_file_name)) {
            std::cerr << "Input file does not exist: " << input_file_name << std::endl;
            continue;
        }
        
        double time = main_encoding_compress(input_file_name, output_file_name,
                                           DefaultParams::WINDOW_SIZE,
                                           DefaultParams::LOG_LENGTH,
                                           DefaultParams::THRESHOLD,
                                           DefaultParams::BLOCK_SIZE,
                                           CompressorType::NONE,
                                           DefaultParams::DISTANCE,
                                           false);  // Use exact
        
        std::cout << "Time cost: " << time << " seconds" << std::endl;
    }

    // Second phase: Process NONE results with other compressors
    std::cout << "\n=== Phase 2: Processing with other compressors ===\n" << std::endl;
    
    std::map<std::string, std::vector<double>> time_sets;

    for (const auto& d : datasets) {
        std::vector<double> time_list;
        std::string none_file = output_path + d + "/" + d + "_NONE";
        
        for (const auto& [comp_name, comp_type] : compressors) {
            std::cout << "\n=== Processing " << d << " with " << comp_name << " ===" << std::endl;
            
            std::string output_file = output_path + d + "/" + d + "_" + comp_name;
            
            if (!std::ifstream(none_file)) {
                std::cerr << "NONE compressed file not found: " << none_file << std::endl;
                continue;
            }
            
            auto start = std::chrono::high_resolution_clock::now();
            bool success = compress_file(none_file, output_file, comp_type);
            auto end = std::chrono::high_resolution_clock::now();
            double time = std::chrono::duration<double>(end - start).count();
            
            if (success) {
                time_list.push_back(time);
                std::cout << "Time cost: " << time << " seconds" << std::endl;
            } else {
                std::cerr << "Compression failed for " << comp_name << std::endl;
                time_list.push_back(-1);
            }
        }
        time_sets[d] = time_list;
    }

    // Write compression times to CSV
    std::string csv_path = output_path + "compression_time.csv";
    std::vector<std::string> first_column;
    first_column.push_back("Compressor");
    for (const auto& [name, _] : compressors) {
        first_column.push_back(name);
    }
    
    bool write_success = write_csv(csv_path, time_sets, datasets, first_column);
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
