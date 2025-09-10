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
    {"ZSTD", CompressorType::ZSTD},
    {"LZ4", CompressorType::LZ4}
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
    std::string input_path = "../../datasets/test_dataset/";
    std::string base_output_path = "../result_approx/";

    std::string lzma_output_path = base_output_path + "lzma/" ;
    std::string zstd_output_path = base_output_path + "zstd/" + "level_3/";
    std::string lz4_output_path = base_output_path + "lz4/" + "level_9/";

    std::cout << "\n=== Processing datasets with different compressors ===\n" << std::endl;
    
    std::map<std::string, std::vector<double>> time_sets;

    for (const auto& d : datasets) {
        std::cout << "\n=== Processing dataset: " << d << " ===" << std::endl;
        std::vector<double> time_list;
        std::string input_file_name = input_path + d + ".log";
        
        if (!std::ifstream(input_file_name)) {
            std::cerr << "Input file does not exist: " << input_file_name << std::endl;
            time_list = {-1, -1, -1};  // All compressors failed
            time_sets[d] = time_list;
            continue;
        }

        // Process with LZMA
        
        if (!ensure_directory_exists(lzma_output_path)) {
            std::cerr << "Failed to create directory: " << lzma_output_path << std::endl;
            time_list.push_back(-1);
        } else {
            std::string lzma_output_file = lzma_output_path + d;
            
            auto start = std::chrono::high_resolution_clock::now();
            double time = main_encoding_compress(input_file_name, lzma_output_file,
                                               DefaultParams::WINDOW_SIZE,
                                               DefaultParams::THRESHOLD,
                                               DefaultParams::BLOCK_SIZE,
                                               CompressorType::LZMA,
                                               DefaultParams::DISTANCE,
                                               DefaultParams::USE_APPROX);
            auto end = std::chrono::high_resolution_clock::now();
            double total_time = std::chrono::duration<double>(end - start).count();
            
            if (time >= 0) {
                time_list.push_back(total_time);
                std::cout << "LZMA processing completed - Total time: " << total_time << " seconds" << std::endl;
            } else {
                std::cerr << "LZMA processing failed" << std::endl;
                time_list.push_back(-1);
            }
        }

        // Process with ZSTD

        if (!ensure_directory_exists(zstd_output_path)) {
            std::cerr << "Failed to create directory: " << zstd_output_path << std::endl;
            time_list.push_back(-1);
        } else {
            std::string zstd_output_file = zstd_output_path + d;
            
            auto start = std::chrono::high_resolution_clock::now();
            double time = main_encoding_compress(input_file_name, zstd_output_file,
                                               DefaultParams::WINDOW_SIZE,
                                               DefaultParams::THRESHOLD,
                                               DefaultParams::BLOCK_SIZE,
                                               CompressorType::ZSTD,
                                               DefaultParams::DISTANCE,
                                               DefaultParams::USE_APPROX);
            auto end = std::chrono::high_resolution_clock::now();
            double total_time = std::chrono::duration<double>(end - start).count();
            
            if (time >= 0) {
                time_list.push_back(total_time);
                std::cout << "ZSTD processing completed - Total time: " << total_time << " seconds" << std::endl;
            } else {
                std::cerr << "ZSTD processing failed" << std::endl;
                time_list.push_back(-1);
            }
        }

        // Process with LZ4

        if (!ensure_directory_exists(lz4_output_path)) {
            std::cerr << "Failed to create directory: " << lz4_output_path << std::endl;
            time_list.push_back(-1);
        } else {
            std::string lz4_output_file = lz4_output_path + d;
            
            auto start = std::chrono::high_resolution_clock::now();
            double time = main_encoding_compress(input_file_name, lz4_output_file,
                                               DefaultParams::WINDOW_SIZE,
                                               DefaultParams::THRESHOLD,
                                               DefaultParams::BLOCK_SIZE,
                                               CompressorType::LZ4,
                                               DefaultParams::DISTANCE,
                                               DefaultParams::USE_APPROX);
            auto end = std::chrono::high_resolution_clock::now();
            double total_time = std::chrono::duration<double>(end - start).count();
            
            if (time >= 0) {
                time_list.push_back(total_time);
                std::cout << "LZ4 processing completed - Total time: " << total_time << " seconds" << std::endl;
            } else {
                std::cerr << "LZ4 processing failed" << std::endl;
                time_list.push_back(-1);
            }
        }

        time_sets[d] = time_list;
    }

    // Write compression times to separate CSV files for each compressor
    std::vector<std::string> column_names = {"Compression_Time"};
    
    // Write LZMA results
    std::string lzma_csv_path = lzma_output_path + "/time_cost.csv";
    std::map<std::string, std::vector<double>> lzma_time_sets;
    std::vector<double> lzma_times;
    for (const auto& d : datasets) {
        if (time_sets.find(d) != time_sets.end() && time_sets[d].size() > 0) {
            lzma_times.push_back(time_sets[d][0]); // LZMA is first in the list
        } else {
            lzma_times.push_back(-1); // Failed case
        }
    }
    lzma_time_sets["Compression_Time"] = lzma_times;
    bool lzma_write_success = write_csv(lzma_csv_path, lzma_time_sets, column_names, datasets);
    
    // Write ZSTD results
    std::string zstd_csv_path = zstd_output_path + "/time_cost.csv";
    std::map<std::string, std::vector<double>> zstd_time_sets;
    std::vector<double> zstd_times;
    for (const auto& d : datasets) {
        if (time_sets.find(d) != time_sets.end() && time_sets[d].size() > 1) {
            zstd_times.push_back(time_sets[d][1]); // ZSTD is second in the list
        } else {
            zstd_times.push_back(-1); // Failed case
        }
    }
    zstd_time_sets["Compression_Time"] = zstd_times;
    bool zstd_write_success = write_csv(zstd_csv_path, zstd_time_sets, column_names, datasets);
    
    // Write LZ4 results
    std::string lz4_csv_path = lz4_output_path + "/time_cost.csv";
    std::map<std::string, std::vector<double>> lz4_time_sets;
    std::vector<double> lz4_times;
    for (const auto& d : datasets) {
        if (time_sets.find(d) != time_sets.end() && time_sets[d].size() > 2) {
            lz4_times.push_back(time_sets[d][2]); // LZ4 is third in the list
        } else {
            lz4_times.push_back(-1); // Failed case
        }
    }
    lz4_time_sets["Compression_Time"] = lz4_times;
    bool lz4_write_success = write_csv(lz4_csv_path, lz4_time_sets, column_names, datasets);
    
    cleanup_resources(time_sets);
    cleanup_resources(lzma_time_sets);
    cleanup_resources(zstd_time_sets);
    cleanup_resources(lz4_time_sets);

    if (lzma_write_success && zstd_write_success && lz4_write_success) {
        std::cout << "\nAll tasks completed. Results written to separate CSV files:" << std::endl;
        std::cout << "LZMA results saved in: " << lzma_csv_path << std::endl;
        std::cout << "ZSTD results saved in: " << zstd_csv_path << std::endl;
        std::cout << "LZ4 results saved in: " << lz4_csv_path << std::endl;
    } else {
        std::cerr << "\nSome CSV files failed to write:" << std::endl;
        if (!lzma_write_success) std::cerr << "LZMA CSV write failed" << std::endl;
        if (!zstd_write_success) std::cerr << "ZSTD CSV write failed" << std::endl;
        if (!lz4_write_success) std::cerr << "LZ4 CSV write failed" << std::endl;
    }
}

void exact_encoding() {
    std::string input_path = "../datasets/test_dataset/";
    std::string base_output_path = "../result_exact/";

    std::string lzma_output_path = base_output_path + "lzma/" ;
    std::string zstd_output_path = base_output_path + "zstd/" + "level_1/";
    std::string lz4_output_path = base_output_path + "lz4/" + "level_1/";

    std::cout << "\n=== Processing datasets with exact encoding ===\n" << std::endl;
    
    std::map<std::string, std::vector<double>> time_sets;

    for (const auto& d : datasets) {
        std::cout << "\n=== Processing dataset: " << d << " ===" << std::endl;
        std::vector<double> time_list;
        std::string input_file_name = input_path + d + ".log";
        
        if (!std::ifstream(input_file_name)) {
            std::cerr << "Input file does not exist: " << input_file_name << std::endl;
            time_list = {-1, -1, -1};  // All compressors failed
            time_sets[d] = time_list;
            continue;
        }

        // Process with LZMA
    
        if (!ensure_directory_exists(lzma_output_path)) {
            std::cerr << "Failed to create directory: " << lzma_output_path << std::endl;
            time_list.push_back(-1);
        } else {
            std::string lzma_output_file = lzma_output_path + d + "_LZMA";
            
            auto start = std::chrono::high_resolution_clock::now();
            double time = main_encoding_compress(input_file_name, lzma_output_file,
                                               DefaultParams::WINDOW_SIZE,
                                               DefaultParams::THRESHOLD,
                                               DefaultParams::BLOCK_SIZE,
                                               CompressorType::LZMA,
                                               DefaultParams::DISTANCE,
                                               false);  // Set use_approx to false
            auto end = std::chrono::high_resolution_clock::now();
            double total_time = std::chrono::duration<double>(end - start).count();
            
            if (time >= 0) {
                time_list.push_back(total_time);
                std::cout << "LZMA processing completed - Total time: " << total_time << " seconds" << std::endl;
            } else {
                std::cerr << "LZMA processing failed" << std::endl;
                time_list.push_back(-1);
            }
        }

        // Process with ZSTD
        if (!ensure_directory_exists(zstd_output_path)) {
            std::cerr << "Failed to create directory: " << zstd_output_path << std::endl;
            time_list.push_back(-1);
        } else {
            std::string zstd_output_file = zstd_output_path + d + "_ZSTD";
            
            auto start = std::chrono::high_resolution_clock::now();
            double time = main_encoding_compress(input_file_name, zstd_output_file,
                                               DefaultParams::WINDOW_SIZE,
                                               DefaultParams::THRESHOLD,
                                               DefaultParams::BLOCK_SIZE,
                                               CompressorType::ZSTD,
                                               DefaultParams::DISTANCE,
                                               false);  // Set use_approx to false
            auto end = std::chrono::high_resolution_clock::now();
            double total_time = std::chrono::duration<double>(end - start).count();
            
            if (time >= 0) {
                time_list.push_back(total_time);
                std::cout << "ZSTD processing completed - Total time: " << total_time << " seconds" << std::endl;
            } else {
                std::cerr << "ZSTD processing failed" << std::endl;
                time_list.push_back(-1);
            }
        }

        // Process with LZ4
        if (!ensure_directory_exists(lz4_output_path)) {
            std::cerr << "Failed to create directory: " << lz4_output_path << std::endl;
            time_list.push_back(-1);
        } else {
            std::string lz4_output_file = lz4_output_path + d + "_LZ4";
            
            auto start = std::chrono::high_resolution_clock::now();
            double time = main_encoding_compress(input_file_name, lz4_output_file,
                                               DefaultParams::WINDOW_SIZE,
                                               DefaultParams::THRESHOLD,
                                               DefaultParams::BLOCK_SIZE,
                                               CompressorType::LZ4,
                                               DefaultParams::DISTANCE,
                                               false);  // Set use_approx to false
            auto end = std::chrono::high_resolution_clock::now();
            double total_time = std::chrono::duration<double>(end - start).count();
            
            if (time >= 0) {
                time_list.push_back(total_time);
                std::cout << "LZ4 processing completed - Total time: " << total_time << " seconds" << std::endl;
            } else {
                std::cerr << "LZ4 processing failed" << std::endl;
                time_list.push_back(-1);
            }
        }

        time_sets[d] = time_list;
    }

    // Write compression times to separate CSV files for each compressor
    std::vector<std::string> column_names = {"Compression_Time"};
    
    // Write LZMA results
    std::string lzma_csv_path = lzma_output_path + "/time_cost.csv";
    std::map<std::string, std::vector<double>> lzma_time_sets;
    std::vector<double> lzma_times;
    for (const auto& d : datasets) {
        if (time_sets.find(d) != time_sets.end() && time_sets[d].size() > 0) {
            lzma_times.push_back(time_sets[d][0]); // LZMA is first in the list
        } else {
            lzma_times.push_back(-1); // Failed case
        }
    }
    lzma_time_sets["Compression_Time"] = lzma_times;
    bool lzma_write_success = write_csv(lzma_csv_path, lzma_time_sets, column_names, datasets);
    
    // Write ZSTD results
    std::string zstd_csv_path = zstd_output_path + "/time_cost.csv";
    std::map<std::string, std::vector<double>> zstd_time_sets;
    std::vector<double> zstd_times;
    for (const auto& d : datasets) {
        if (time_sets.find(d) != time_sets.end() && time_sets[d].size() > 1) {
            zstd_times.push_back(time_sets[d][1]); // ZSTD is second in the list
        } else {
            zstd_times.push_back(-1); // Failed case
        }
    }
    zstd_time_sets["Compression_Time"] = zstd_times;
    bool zstd_write_success = write_csv(zstd_csv_path, zstd_time_sets, column_names, datasets);
    
    // Write LZ4 results
    std::string lz4_csv_path = base_output_path + "lz4/time_cost.csv";
    std::map<std::string, std::vector<double>> lz4_time_sets;
    std::vector<double> lz4_times;
    for (const auto& d : datasets) {
        if (time_sets.find(d) != time_sets.end() && time_sets[d].size() > 2) {
            lz4_times.push_back(time_sets[d][2]); // LZ4 is third in the list
        } else {
            lz4_times.push_back(-1); // Failed case
        }
    }
    lz4_time_sets["Compression_Time"] = lz4_times;
    bool lz4_write_success = write_csv(lz4_csv_path, lz4_time_sets, column_names, datasets);
    
    cleanup_resources(time_sets);
    cleanup_resources(lzma_time_sets);
    cleanup_resources(zstd_time_sets);
    cleanup_resources(lz4_time_sets);

    if (lzma_write_success && zstd_write_success && lz4_write_success) {
        std::cout << "\nAll tasks completed. Results written to separate CSV files:" << std::endl;
        std::cout << "LZMA results saved in: " << lzma_csv_path << std::endl;
        std::cout << "ZSTD results saved in: " << zstd_csv_path << std::endl;
        std::cout << "LZ4 results saved in: " << lz4_csv_path << std::endl;
    } else {
        std::cerr << "\nSome CSV files failed to write:" << std::endl;
        if (!lzma_write_success) std::cerr << "LZMA CSV write failed" << std::endl;
        if (!zstd_write_success) std::cerr << "ZSTD CSV write failed" << std::endl;
        if (!lz4_write_success) std::cerr << "LZ4 CSV write failed" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    try {
        std::cout << "\n=== Starting Approximate Encoding ===\n" << std::endl;
        approx_encoding();
        std::cout << "\n=== Approximate Encoding Completed ===\n" << std::endl;
        
        // Add a small delay to ensure resources are properly released
        std::cout.flush();
        
        // std::cout << "\n=== Starting Exact Encoding ===\n" << std::endl;
        // exact_encoding();
        // std::cout << "\n=== Exact Encoding Completed ===\n" << std::endl;

        std::cout << "\n=== All Tasks Completed ===\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
}
