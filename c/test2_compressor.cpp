#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <chrono>
#include "record_compress.hpp"
#include "utils.hpp"
#include "bit_buffer.hpp"
#include <stdexcept>

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
    // Use BitCompressor's static compress_file method
    if (!BitCompressor::compress_file(input_file, output_file, comp_type)) {
        std::cerr << "Failed to compress file: " << input_file << std::endl;
        return false;
    }
    
    return true;
}

void approx_encoding() {
    std::string input_path = "../datasets/test_dataset/";
    std::string output_path = "../result_new/result_approx/test2_compressor/";

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
                                           DefaultParams::THRESHOLD,
                                           DefaultParams::BLOCK_SIZE,
                                           CompressorType::NONE,
                                           DefaultParams::DISTANCE,
                                           DefaultParams::USE_APPROX);
        
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
    std::string csv_path = output_path + "time_cost.csv";
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
    std::string output_path = "../result_new/result_exact/test2_compressor/";

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
                                           DefaultParams::THRESHOLD,
                                           DefaultParams::BLOCK_SIZE,
                                           CompressorType::NONE,
                                           DefaultParams::DISTANCE,
                                           false);  // Set use_approx to false
        
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
    std::string csv_path = output_path + "time_cost.csv";
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

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file> <compressor_type>" << std::endl;
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];
    int compressor_type = std::stoi(argv[3]);

    try {
        // Read input file into BitOutBuffer
        BitOutBuffer out_buffer;
        std::ifstream in_file(input_file, std::ios::binary);
        if (!in_file) {
            throw std::runtime_error("Failed to open input file: " + input_file);
        }

        // Read and encode data
        char buffer[1024];
        while (in_file.read(buffer, sizeof(buffer))) {
            for (size_t i = 0; i < sizeof(buffer); i++) {
                out_buffer.encode(static_cast<uint8_t>(buffer[i]), 8);
            }
        }
        
        // Handle remaining bytes
        size_t remaining = in_file.gcount();
        for (size_t i = 0; i < remaining; i++) {
            out_buffer.encode(static_cast<uint8_t>(buffer[i]), 8);
        }

        // Write compressed output using BitOutBuffer
        if (!out_buffer.write(output_file, "wb", static_cast<CompressorType>(compressor_type))) {
            throw std::runtime_error("Failed to write output file: " + output_file);
        }

        // Verify by reading back
        BitInBuffer in_buffer;
        if (!in_buffer.read(output_file, static_cast<CompressorType>(compressor_type))) {
            throw std::runtime_error("Failed to read compressed file: " + output_file);
        }

        // Create verification file
        std::string verify_file = output_file + ".verify";
        std::ofstream verify_out(verify_file, std::ios::binary);
        if (!verify_out) {
            throw std::runtime_error("Failed to open verification file: " + verify_file);
        }

        // Decode and write data
        try {
            while (true) {
                uint8_t byte = in_buffer.decode(8);
                verify_out.write(reinterpret_cast<char*>(&byte), 1);
            }
        } catch (const std::runtime_error&) {
            // End of stream
        }

        std::cout << "Compression and verification completed successfully" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
