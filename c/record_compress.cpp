#include "rle.hpp"
#include "utils.hpp"
#include "distance.hpp"
#include "bit_buffer.hpp"
#include "bit_packing.hpp"
#include "qgram_match.hpp"
#include "record_compress.hpp"
#include "ts_2diff.hpp"
#include "variable_length_substitution.hpp"
#include <chrono>
#include <deque>
#include <fstream>
#include <iostream>
#include <bitset>
#include <stdexcept>
#include <algorithm>
#include <iomanip>

// Define macro for encoding statistics output
#ifndef ENCODING_STATS
#define ENCODING_STATS 0  // Set to 1 to enable statistics output
#endif

#define PRINT_STATS(x) if (ENCODING_STATS) { std::cout << x << std::endl; }

void printRecord(const Record& record, int idx) {
    std::cout << "Record[" << idx << "]: method=" << record.method
            //   << ", another_line=" << record.another_line
              << ", begin=" << record.begin
              << ", operation_size=" << record.operation_size;
    std::cout << "  position_list: ";
    for (auto v : record.position_list) std::cout << v << " ";
    std::cout << "  d_length: ";
    for (auto v : record.d_length) std::cout << v << " ";
    std::cout << "  i_length: ";
    for (auto v : record.i_length) std::cout << v << " ";
    std::cout << "  sub_string: ";
    for (auto& s : record.sub_string) std::cout << '"' << s << '"' << " ";
    std::cout << std::endl;
}

void byteArrayEncoding(const std::vector<Record>& records, const std::string& output_path, CompressorType compressor) {
    BitOutBuffer stream;

    // Separate records with method 0 and 1
    std::vector<Record> records0, records1;
    for (const auto& record : records) {
        if (record.method == 0) records0.push_back(record);
        else records1.push_back(record);
    }

    PRINT_STATS("\n=== Block Encoding Statistics ===");
    PRINT_STATS("Records0 (method 0) count: " << records0.size());
    PRINT_STATS("Records1 (method 1) count: " << records1.size());
    PRINT_STATS("Total records: " << records.size());

    // Encode record counts
    stream.encode(records0.size(), 32);
    stream.encode(records1.size(), 32);
    PRINT_STATS("Record counts encoding length: " << 8 << " bytes");

    // Encode method using RLE
    std::vector<int> method_list;
    for (const auto& record : records) {
        method_list.push_back(record.method);
    }
    auto method_encoded = rleEncode(method_list);
    std::vector<unsigned char> rle_bytes = method_encoded.bytes;
    size_t method_interval_count = method_encoded.interval_count;
    std::string rle_string;
    for (unsigned char byte : rle_bytes) {
        rle_string += std::bitset<8>(byte).to_string();
    }
    size_t method_length = (rle_string.length() + 7) / 8;
    while (rle_string.length() < method_length * 8) {
        rle_string += '0';
    }
    stream.encode(static_cast<int>(method_length), 16);
    for (size_t i = 0; i < method_length; i++) {
        int bf = std::stoi(rle_string.substr(i * 8, 8), nullptr, 2);
        stream.encode(bf, 8);
    }
    stream.encode(method_interval_count, 32);
    PRINT_STATS("Method encoding length: " << (2 + method_length + 4) << " bytes");

    // // Encode another_line using RLE
    // std::vector<int> another_line_list;
    // for (const auto& record : records) {
    //     another_line_list.push_back(record.another_line);
    // }
    // auto line_encoded = rleEncode(another_line_list);
    // rle_bytes = line_encoded.bytes;
    // size_t line_interval_count = line_encoded.interval_count;
    // rle_string.clear();
    // for (unsigned char byte : rle_bytes) {
    //     rle_string += std::bitset<8>(byte).to_string();
    // }
    // size_t line_length = (rle_string.length() + 7) / 8;
    // while (rle_string.length() < line_length * 8) {
    //     rle_string += '0';
    // }
    // stream.encode(static_cast<int>(line_length), 16);
    // for (size_t i = 0; i < line_length; i++) {
    //     int bf = std::stoi(rle_string.substr(i * 8, 8), nullptr, 2);
    //     stream.encode(bf, 8);
    // }
    // stream.encode(line_interval_count, 32);
    // PRINT_STATS("Line encoding length: " << (2 + line_length + 4) << " bytes");

    // Encode begin using bit packing
    std::vector<int> begins;
    for (const auto& record : records0) {
        begins.push_back(record.begin);
    }

    if (begins.empty()) {
        stream.encode(0, 16);
        PRINT_STATS("Begin encoding length: 2 bytes (empty)");
    } else {
        std::vector<unsigned char> packed_bytes = bit_packing_encode(begins);
        std::string bit_packing_string;
        for (unsigned char byte : packed_bytes) {
            bit_packing_string += std::bitset<8>(byte).to_string();
        }
        size_t begin_length = (bit_packing_string.length() + 7) / 8;
        while (bit_packing_string.length() < begin_length * 8) {
            bit_packing_string += '0';
        }
        stream.encode(static_cast<int>(begin_length), 16);
        for (size_t i = 0; i < begin_length; i++) {
            int bf = std::stoi(bit_packing_string.substr(i * 8, 8), nullptr, 2);
            stream.encode(bf, 8);
        }
        PRINT_STATS("Begin encoding length: " << (2 + begin_length) << " bytes");
    }

    // Encode operation_size using bit packing
    std::vector<int> operation_sizes;
    for (const auto& record : records0) {
        operation_sizes.push_back(record.operation_size);
    }

    if (operation_sizes.empty()) {
        stream.encode(0, 16);
        PRINT_STATS("Operation size encoding length: 2 bytes (empty)");
    } else {
        std::vector<unsigned char> packed_bytes = bit_packing_encode(operation_sizes);
        std::string bit_packing_string;
        for (unsigned char byte : packed_bytes) {
            bit_packing_string += std::bitset<8>(byte).to_string();
        }
        size_t operation_length = (bit_packing_string.length() + 7) / 8;
        while (bit_packing_string.length() < operation_length * 8) {
            bit_packing_string += '0';
        }
        stream.encode(static_cast<int>(operation_length), 16);
        for (size_t i = 0; i < operation_length; i++) {
            int bf = std::stoi(bit_packing_string.substr(i * 8, 8), nullptr, 2);
            stream.encode(bf, 8);
        }
        PRINT_STATS("Operation size encoding length: " << (2 + operation_length) << " bytes");
    }

    // Encode lengths
    std::vector<int> length_list;
    for (const auto& record : records0) {
        length_list.insert(length_list.end(), record.d_length.begin(), record.d_length.end());
        length_list.insert(length_list.end(), record.i_length.begin(), record.i_length.end());
    }

    size_t length_encoding_size = 0;
    if (!records0.empty()) {
        length_encoding_size = ts2diff_encode(length_list, stream);
        PRINT_STATS("Length encoding size: " << length_encoding_size << " bytes");
    } else {
        PRINT_STATS("Length encoding size: 0 bytes (empty)");
    }

    // Encode positions
    size_t position_encoding_size = 0;
    if (!records0.empty()) {
        std::vector<int> p_begin_list;
        std::vector<int> p_delta_list;

        for (const auto& record : records0) {
            int oldp = -1;
            for (int p : record.position_list) {
                if (oldp == -1) {
                    p_begin_list.push_back(p);
                } else {
                    p_delta_list.push_back(p - oldp);
                }
                oldp = p;
            }
        }

        // Use ts2diff to encode begin positions
        if (!p_begin_list.empty()) {
            size_t begin_pos_size = ts2diff_encode(p_begin_list, stream);
            position_encoding_size += begin_pos_size;
            PRINT_STATS("Begin positions encoding size: " << begin_pos_size << " bytes");
        }

        // Use ts2diff to encode delta positions
        if (!p_delta_list.empty()) {
            size_t delta_pos_size = ts2diff_encode(p_delta_list, stream);
            position_encoding_size += delta_pos_size;
            PRINT_STATS("Delta positions encoding size: " << delta_pos_size << " bytes");
        }
    } else {
        PRINT_STATS("Position encoding size: 0 bytes (empty)");
    }

    // Encode strings
    std::string sub_string;
    for (const auto& record : records0) {
        for (const auto& s : record.sub_string) {
            sub_string += s;
        }
    }
    for (const auto& record : records1) {
        for (const auto& s : record.sub_string) {
            sub_string += s;
            sub_string += '\n';  // Add newline for each string of record1
        }
    }

    // Encode strings
    for (char byte : sub_string) {
        stream.encode(static_cast<unsigned char>(byte), 8);
    }
    PRINT_STATS("String encoding size: " << sub_string.length() << " bytes");
    PRINT_STATS("=== End of Block Encoding ===\n");

    stream.write(output_path, "ab");
}

double main_encoding_compress(const std::string& input_path, 
                                   const std::string& output_path,
                                   int window_size,
                                   double threshold, int block_size,
                                   CompressorType compressor,
                                   DistanceType distance,
                                   bool use_approx) {
    auto total_start_time = std::chrono::high_resolution_clock::now();
    
    // Add counters
    size_t total_lines = 0;
    size_t matched_lines = 0;
    
    // Time statistics for each part
    double read_time = 0;
    double distance_time = 0;
    double match_time = 0;
    double encoding_time = 0;
    
    std::deque<std::string> q;
    // int new_line_flag = 0;
    std::ifstream input(input_path, std::ios::binary);
    
    // Write encoding head
    BitOutBuffer stream;
    stream.encode(window_size, 16);
    // stream.encode(block_size, 16);
    
    // Write parameter byte
    int compressor_val = static_cast<int>(compressor);
    int distance_val = static_cast<int>(distance);
    uint8_t param_byte = (compressor_val & 0xF) | ((distance_val & 0x7) << 4) | ((use_approx ? 1 : 0) << 7);
    stream.encode(param_byte, 8);
    stream.write(output_path, "wb");

    bool loop_end = false;
    int block_cnt = 0;

    while (!loop_end) {
        // Clear MinHash cache at the start of each block
        std::vector<Record> records;

        MinHash::getInstance().clearCache();
        
        std::vector<int> line_flag;
        std::vector<std::string> line_list;
        int index = 0;


        auto read_start = std::chrono::high_resolution_clock::now();
        // // Read data block
        // auto read_start = std::chrono::high_resolution_clock::now();
        while (index < block_size) {
            std::string line;
            if (!std::getline(input, line)) {
                loop_end = true;
                break;
            }
            line_list.push_back(line);
            index++;
        }
        auto read_end = std::chrono::high_resolution_clock::now();
        read_time += std::chrono::duration<double>(read_end - read_start).count();

        // Output the first 20 records
        // std::cout << "\nFirst 20 lines in line_list:" << std::endl;
        // for (int i = 0; i < std::min(20, (int)line_list.size()); i++) {
        //     std::cout << "Line " << i << ": " << line_list[i] << std::endl;
        // }
        // std::cout << std::endl;

        // Process each line
        int id = 0;
        for (const auto& line : line_list) {
            total_lines++;
            int begin = -1;

            auto distance_start = std::chrono::high_resolution_clock::now();

            // Calculate distances
            double min_distance = 1.0;  // Initialize to maximum distance
            for (size_t i = 0; i < q.size(); i++) {
                double tmp_dist = Distance::calculateDistance(q[i], line, distance, DefaultParams::Q);
                if (tmp_dist < min_distance) {
                    min_distance = tmp_dist;
                    begin = static_cast<int>(i);
                }
            }
            
            // Since all distances are now in [0,1], we can use threshold directly
            if (min_distance >= threshold) {
                begin = -1;
            }

            auto distance_end = std::chrono::high_resolution_clock::now();
            distance_time += std::chrono::duration<double>(distance_end - distance_start).count();

            auto match_start = std::chrono::high_resolution_clock::now();
            Record record;
            if (begin == -1) {
                record.method = 1;
                record.sub_string.push_back(line);
            } else {
                matched_lines++;
                // Choose matching algorithm based on use_approx parameter
                std::vector<OperationItem> op_list;
                double new_distance;
                if (use_approx) {
                    // Use approximate algorithm with default Q
                    std::tie(op_list, new_distance) = getQgramMatchOplist(q[begin], line, DefaultParams::Q);
                } else {
                    // Use exact algorithm
                    std::tie(op_list, new_distance) = getSubstitutionOplist(q[begin], line);
                }
                
                if (new_distance > line.length()) {
                    record.method = 1;
                    record.sub_string.push_back(line);
                } else {
                    record.method = 0;
                    record.begin = begin;
                    record.operation_size = op_list.size();
                    for (const auto& op : op_list) {
                        record.position_list.push_back(op.position);
                        record.d_length.push_back(op.length1);
                        record.i_length.push_back(op.length2);
                        record.sub_string.push_back(op.substr);
                    }
                }
            }
            records.push_back(record);
            // printRecord(record, id);

            id++;

            auto match_end = std::chrono::high_resolution_clock::now();
            match_time += std::chrono::duration<double>(match_end - match_start).count();

            // Update sliding window
            if (q.size() < static_cast<size_t>(window_size)) {
                q.push_back(line);
            } else {
                q.pop_front();
                q.push_back(line);
            }
        }

        // // Print all records in this block
        // std::cout << "\n=== Records in this block (total: " << records.size() << ") ===" << std::endl;
        // for (size_t i = 0; i < records.size(); ++i) {
        //     printRecord(records[i], i);
        // }
        // std::cout << "=== End of block records ===\n" << std::endl;

        // Encode records
        auto encoding_start = std::chrono::high_resolution_clock::now();
        // byteArrayEncoding(records, output_path, compressor);
        byteArrayEncoding(records, output_path, CompressorType::NONE);
        auto encoding_end = std::chrono::high_resolution_clock::now();
        encoding_time += std::chrono::duration<double>(encoding_end - encoding_start).count();

        block_cnt++;
    }

    auto total_end_time = std::chrono::high_resolution_clock::now();
    double total_time = std::chrono::duration<double>(total_end_time - total_start_time).count();

    // Secondary compression using specified compressor
    auto comp_start = std::chrono::high_resolution_clock::now();
    if (!BitCompressor::compress_file(output_path, output_path, compressor)) {
        throw std::runtime_error("Failed to compress output file: " + output_path);
    }
    auto comp_end = std::chrono::high_resolution_clock::now();
    double comp_time = std::chrono::duration<double>(comp_end - comp_start).count();

    // Print time statistics
    // std::cout << "Time statistics:" << std::endl;
    // std::cout << "  Read time: " << read_time << " seconds" << std::endl;
    // std::cout << "  Distance calculation time: " << distance_time << " seconds" << std::endl;
    // std::cout << "  Q-gram matching time: " << match_time << " seconds" << std::endl;
    // std::cout << "  Encoding time: " << encoding_time << " seconds" << std::endl;
    // std::cout << "  Compressor compression time: " << comp_time << " seconds" << std::endl;
    std::cout << "Compressing Total time: " << total_time + comp_time << " seconds" << std::endl;

    // // Print matching statistics
    // std::cout << "\nMatching statistics:" << std::endl;
    // std::cout << "  Total lines processed: " << total_lines << std::endl;
    // std::cout << "  Lines matched: " << matched_lines << std::endl;
    // std::cout << "  Match rate: " << (100.0 * matched_lines / total_lines) << "%" << std::endl;

    // return total_time + comp_time;
    return total_time + comp_time;
}

#if defined(RECORD_COMPRESS) && !defined(TEST_MODE)
int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input_path> <output_path> [compressor] [window_size] [log_length] [threshold] [block_size] [distance] [use_approx]" << std::endl;
        std::cerr << "Compressor options: none, lzma, gzip, zstd" << std::endl;
        std::cerr << "Distance options: cosine, minhash, qgram" << std::endl;
        std::cerr << "Use approx options: true, false (default: true)" << std::endl;
        return 1;
    }

    std::string input_path = argv[1];
    std::string output_path = argv[2];
    
    // Default parameters
    std::string compressor_setting = "none";
    int window_size = 8;
    double threshold = 0.06;
    int block_size = 327680000;
    std::string distance_setting = "minhash";

    CompressorType compressor = CompressorType::NONE;
    DistanceType distance = DistanceType::MINHASH;

    // Optional parameters with bounds checking
    if (argc > 3 && argv[3] != nullptr) {
        compressor_setting = argv[3];
    }
    
    // Set compressor type
    if (compressor_setting == "lzma") {
        compressor = CompressorType::LZMA;
    } else if (compressor_setting == "gzip") {
        compressor = CompressorType::GZIP;
    } else if (compressor_setting == "zstd") {
        compressor = CompressorType::ZSTD;
    } else {
        compressor = CompressorType::NONE;
    }

    // Parse numeric parameters
    if (argc > 4 && argv[4] != nullptr) {
        try {
            std::string arg(argv[4]);
            if (!arg.empty()) {
                window_size = std::stoi(arg);
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid window size parameter: " << argv[4] << std::endl;
            return 1;
        }
    }
    
    
    if (argc > 5 && argv[5] != nullptr) {
        try {
            std::string arg(argv[5]);
            if (!arg.empty()) {
                threshold = std::stod(arg);
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid threshold parameter: " << argv[5] << std::endl;
            return 1;
        }
    }
    
    if (argc > 6 && argv[6] != nullptr) {
        try {
            std::string arg(argv[6]);
            if (!arg.empty()) {
                block_size = std::stoi(arg);
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid block size parameter: " << argv[6] << std::endl;
            return 1;
        }
    }

    // Parse distance function parameter
    if (argc > 7 && argv[7] != nullptr) {
        distance_setting = argv[7];
    }

    // Set distance type
    if (distance_setting == "cosine") {
        distance = DistanceType::COSINE;
    } else if (distance_setting == "qgram") {
        distance = DistanceType::QGRAM;
    } else {
        distance = DistanceType::MINHASH;  // default
    }

    // Add use_approx parameter
    bool use_approx = true;  // Default is true
    if (argc > 8 && argv[8] != nullptr) {
        std::string approx_setting = argv[8];
        if (approx_setting == "false") {
            use_approx = false;
        }
    }

    // Print parameters for verification
    std::cout << "\nUsing parameters:" << std::endl;
    std::cout << "  Compressor: " << compressor_setting << std::endl;
    std::cout << "  Window size: " << window_size << std::endl;
    std::cout << "  Threshold: " << threshold << std::endl;
    std::cout << "  Block size: " << block_size << std::endl;
    std::cout << "  Distance function: " << distance_setting << std::endl;
    std::cout << "  Use approximation: " << (use_approx ? "true" : "false") << std::endl;

    try {
        main_encoding_compress(
            input_path, 
            output_path, 
            window_size, 
            threshold, 
            block_size,
            compressor,
            distance,
            use_approx
        );
        
        // std::cout << "Compression completed in " << time_cost << " seconds." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
#endif // RECORD_COMPRESS