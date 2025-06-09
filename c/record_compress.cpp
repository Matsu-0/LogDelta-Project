#include "rle.hpp"
#include "utils.hpp"
#include "distance.hpp"
#include "bit_buffer.hpp"
#include "bit_packing.hpp"
#include "qgram_match.hpp"
#include "record_compress.hpp"
#include "variable_length_substitution.hpp"
#include <chrono>
#include <deque>
#include <fstream>
#include <iostream>
#include <bitset>
#include <stdexcept>

void byteArrayEncoding(const std::vector<Record>& records, const std::string& output_path, CompressorType compressor) {
    const int encoding_block = 1024;
    BitOutBuffer stream;

    // Separate records with method 0 and 1
    std::vector<Record> records0, records1;
    for (const auto& record : records) {
        if (record.method == 0) records0.push_back(record);
        else records1.push_back(record);
    }

    stream.encode(records0.size(), 16);
    stream.encode(records1.size(), 16);

    // Encode method using RLE
    std::vector<int> method_list;
    for (const auto& record : records) {
        method_list.push_back(record.method);
    }
    std::vector<unsigned char> rle_bytes = rleEncode(method_list);
    std::string rle_string;
    for (unsigned char byte : rle_bytes) {
        rle_string += std::bitset<8>(byte).to_string();
    }
    int method_length = (rle_string.length() + 7) / 8;
    while (rle_string.length() < method_length * 8) {
        rle_string += '0';
    }
    stream.encode(method_length, 16);
    for (int i = 0; i < method_length; i++) {
        int bf = std::stoi(rle_string.substr(i * 8, 8), nullptr, 2);
        stream.encode(bf, 8);
    }

    // Encode another_line using RLE
    std::vector<int> another_line_list;
    for (const auto& record : records) {
        another_line_list.push_back(record.another_line);
    }
    rle_bytes = rleEncode(another_line_list);
    rle_string.clear();
    for (unsigned char byte : rle_bytes) {
        rle_string += std::bitset<8>(byte).to_string();
    }
    int line_length = (rle_string.length() + 7) / 8;
    while (rle_string.length() < line_length * 8) {
        rle_string += '0';
    }
    stream.encode(line_length, 16);
    for (int i = 0; i < line_length; i++) {
        int bf = std::stoi(rle_string.substr(i * 8, 8), nullptr, 2);
        stream.encode(bf, 8);
    }

    // Encode begin using bit packing
    std::vector<int> begins;
    for (const auto& record : records0) {
        begins.push_back(record.begin);
    }

    if (begins.empty()) {
        stream.encode(0, 16);
    } else {
        std::vector<unsigned char> packed_bytes = bit_packing_encode(begins);
        std::string bit_packing_string;
        for (unsigned char byte : packed_bytes) {
            bit_packing_string += std::bitset<8>(byte).to_string();
        }
        int leng = (bit_packing_string.length() + 7) / 8;
        while (bit_packing_string.length() < leng * 8) {
            bit_packing_string += '0';
        }
        stream.encode(leng, 16);
        for (int i = 0; i < leng; i++) {
            int bf = std::stoi(bit_packing_string.substr(i * 8, 8), nullptr, 2);
            stream.encode(bf, 8);
        }
    }

    // Encode operation_size using bit packing
    std::vector<int> operation_sizes;
    for (const auto& record : records0) {
        operation_sizes.push_back(record.operation_size);
    }

    if (operation_sizes.empty()) {
        stream.encode(0, 16);
    } else {
        std::vector<unsigned char> packed_bytes = bit_packing_encode(operation_sizes);
        std::string bit_packing_string;
        for (unsigned char byte : packed_bytes) {
            bit_packing_string += std::bitset<8>(byte).to_string();
        }
        int leng = (bit_packing_string.length() + 7) / 8;
        while (bit_packing_string.length() < leng * 8) {
            bit_packing_string += '0';
        }
        stream.encode(leng, 16);
        for (int i = 0; i < leng; i++) {
            int bf = std::stoi(bit_packing_string.substr(i * 8, 8), nullptr, 2);
            stream.encode(bf, 8);
        }
    }

    // Encode lengths
    std::vector<int> length_list;
    for (const auto& record : records0) {
        length_list.insert(length_list.end(), record.d_length.begin(), record.d_length.end());
        length_list.insert(length_list.end(), record.i_length.begin(), record.i_length.end());
    }

    if (length_list.empty()) {
        stream.encode(0, 16);
    } else {
        int blocks = std::max(1, (int)((length_list.size() - 1) / encoding_block));
        for (int i = 0; i < blocks; i++) {
            auto start = length_list.begin() + i * encoding_block;
            auto end = start + std::min(encoding_block, (int)(length_list.end() - start));
            std::vector<int> block_list(start, end);
            
            std::vector<unsigned char> packed_bytes = bit_packing_encode(block_list);
            std::string bit_packing_string;
            for (unsigned char byte : packed_bytes) {
                bit_packing_string += std::bitset<8>(byte).to_string();
            }
            int leng = (bit_packing_string.length() + 7) / 8;
            while (bit_packing_string.length() < leng * 8) {
                bit_packing_string += '0';
            }
            stream.encode(leng, 16);
            for (int j = 0; j < leng; j++) {
                int bf = std::stoi(bit_packing_string.substr(j * 8, 8), nullptr, 2);
                stream.encode(bf, 8);
            }
        }
    }

    // Encode positions
    if (records0.empty()) {
        stream.encode(0, 16);
    } else {
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

        int block_num = (p_begin_list.size() + encoding_block - 1) / encoding_block;
        stream.encode(block_num, 16);

        // Encode begin positions
        for (int i = 0; i < block_num; i++) {
            auto start = p_begin_list.begin() + i * encoding_block;
            auto end = start + std::min(encoding_block, (int)(p_begin_list.end() - start));
            std::vector<int> block(start, end);
            
            std::vector<unsigned char> packed_bytes = bit_packing_encode(block);
            std::string bit_packing_string;
            for (unsigned char byte : packed_bytes) {
                bit_packing_string += std::bitset<8>(byte).to_string();
            }
            int leng = (bit_packing_string.length() + 7) / 8;
            while (bit_packing_string.length() < leng * 8) {
                bit_packing_string += '0';
            }
            stream.encode(leng, 16);
            for (int j = 0; j < leng; j++) {
                int bf = std::stoi(bit_packing_string.substr(j * 8, 8), nullptr, 2);
                stream.encode(bf, 8);
            }
        }

        // Encode delta positions
        int delta_blocks = std::max(1, (int)((p_delta_list.size() - 1) / encoding_block));
        for (int i = 0; i < delta_blocks; i++) {
            auto start = p_delta_list.begin() + i * encoding_block;
            auto end = start + std::min(encoding_block, (int)(p_delta_list.end() - start));
            std::vector<int> block(start, end);
            
            std::vector<unsigned char> packed_bytes = bit_packing_encode(block);
            std::string bit_packing_string;
            for (unsigned char byte : packed_bytes) {
                bit_packing_string += std::bitset<8>(byte).to_string();
            }
            int leng = (bit_packing_string.length() + 7) / 8;
            while (bit_packing_string.length() < leng * 8) {
                bit_packing_string += '0';
            }
            stream.encode(leng, 16);
            for (int j = 0; j < leng; j++) {
                int bf = std::stoi(bit_packing_string.substr(j * 8, 8), nullptr, 2);
                stream.encode(bf, 8);
            }
        }
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
        }
    }

    for (char byte : sub_string) {
        stream.encode(static_cast<unsigned char>(byte), 8);
    }

    stream.write(output_path, "ab", compressor);
}

double main_encoding_compress(const std::string& input_path, 
                                   const std::string& output_path,
                                   int window_size, int log_length,
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
    int new_line_flag = 0;
    std::ifstream input(input_path, std::ios::binary);
    
    // Write encoding head
    BitOutBuffer stream;
    stream.encode(window_size, 16);
    stream.encode(log_length, 16);
    stream.encode(block_size, 16);
    // 写入参数字节
    int compressor_val = static_cast<int>(compressor);
    int distance_val = static_cast<int>(distance);
    uint8_t param_byte = (compressor_val & 0xF) | ((distance_val & 0x7) << 4) | ((use_approx ? 1 : 0) << 7);
    stream.encode(param_byte, 8);
    stream.write(output_path);

    std::vector<Record> records;
    bool loop_end = false;
    int block_cnt = 0;

    while (!loop_end) {
        // Clear MinHash cache at the start of each block
        MinHash::getInstance().clearCache();
        
        std::vector<int> line_flag;
        std::vector<std::string> line_list;
        int index = 0;

        // Read data block
        auto read_start = std::chrono::high_resolution_clock::now();
        while (index < block_size) {
            std::string line;
            if (!std::getline(input, line)) {
                loop_end = true;
                break;
            }

            while (line.length() >= log_length) {
                line_list.push_back(line.substr(0, log_length - 1));
                line = line.substr(log_length - 1);
                line_flag.push_back(1);
                index++;
            }

            line_list.push_back(line);
            line_flag.push_back(0);
            index++;
        }
        auto read_end = std::chrono::high_resolution_clock::now();
        read_time += std::chrono::duration<double>(read_end - read_start).count();

        // Process each line
        for (const auto& line : line_list) {
            total_lines++;
            int begin = -1;

            auto distance_start = std::chrono::high_resolution_clock::now();

            // Calculate distances
            double min_distance = 1.0;  // Initialize to maximum distance
            for (int i = 0; i < q.size(); i++) {
                double tmp_dist = Distance::calculateDistance(q[i], line, distance, DefaultParams::Q);
                if (tmp_dist < min_distance) {
                    min_distance = tmp_dist;
                    begin = i;
                }
            }
            
            // Since all distances are now in [0,1], we can use threshold directly
            if (min_distance >= threshold) {
                begin = -1;
            }

            auto distance_end = std::chrono::high_resolution_clock::now();
            distance_time += std::chrono::duration<double>(distance_end - distance_start).count();

            auto match_start = std::chrono::high_resolution_clock::now();
            if (begin == -1) {
                Record record;
                record.method = 1;
                record.another_line = new_line_flag;
                record.sub_string.push_back(line);
                records.push_back(record);
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
                    Record record;
                    record.method = 1;
                    record.another_line = new_line_flag;
                    record.sub_string.push_back(line);
                    records.push_back(record);
                } else {
                    Record record;
                    record.method = 0;
                    record.another_line = new_line_flag;
                    record.begin = begin;
                    record.operation_size = op_list.size();
                    for (const auto& op : op_list) {
                        record.position_list.push_back(op.position);
                        record.d_length.push_back(op.length1);
                        record.i_length.push_back(op.length2);
                        record.sub_string.push_back(op.substr);
                    }
                    records.push_back(record);
                }
            }

            auto match_end = std::chrono::high_resolution_clock::now();
            match_time += std::chrono::duration<double>(match_end - match_start).count();

            // Update sliding window
            if (q.size() < window_size) {
                q.push_back(line);
            } else {
                q.pop_front();
                q.push_back(line);
            }
        }

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

    // // Secondary compression using specified compressor
    // auto comp_start = std::chrono::high_resolution_clock::now();
    // if (!BitCompressor::compress_file(output_path, output_path, compressor)) {
    //     throw std::runtime_error("Failed to compress output file: " + output_path);
    // }
    // auto comp_end = std::chrono::high_resolution_clock::now();
    // double comp_time = std::chrono::duration<double>(comp_end - comp_start).count();

    // // Print time statistics
    // std::cout << "\nTime statistics:" << std::endl;
    // std::cout << "  Read time: " << read_time << " seconds" << std::endl;
    // std::cout << "  Distance calculation time: " << distance_time << " seconds" << std::endl;
    // std::cout << "  Q-gram matching time: " << match_time << " seconds" << std::endl;
    // std::cout << "  Encoding time: " << encoding_time << " seconds" << std::endl;
    // std::cout << "  Compressor compression time: " << comp_time << " seconds" << std::endl;
    // std::cout << "  Total time: " << total_time + comp_time << " seconds" << std::endl;

    // // Print matching statistics
    // std::cout << "\nMatching statistics:" << std::endl;
    // std::cout << "  Total lines processed: " << total_lines << std::endl;
    // std::cout << "  Lines matched: " << matched_lines << std::endl;
    // std::cout << "  Match rate: " << (100.0 * matched_lines / total_lines) << "%" << std::endl;

    // return total_time + comp_time;
    return total_time;
}

#ifdef RECORD_COMPRESS
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
    int log_length = 256;
    double threshold = 0.06;
    int block_size = 32768;
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
                log_length = std::stoi(arg);
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid log length parameter: " << argv[5] << std::endl;
            return 1;
        }
    }
    
    if (argc > 6 && argv[6] != nullptr) {
        try {
            std::string arg(argv[6]);
            if (!arg.empty()) {
                threshold = std::stod(arg);
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid threshold parameter: " << argv[6] << std::endl;
            return 1;
        }
    }
    
    if (argc > 7 && argv[7] != nullptr) {
        try {
            std::string arg(argv[7]);
            if (!arg.empty()) {
                block_size = std::stoi(arg);
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid block size parameter: " << argv[7] << std::endl;
            return 1;
        }
    }

    // Parse distance function parameter
    if (argc > 8 && argv[8] != nullptr) {
        distance_setting = argv[8];
    }

    // Set distance type
    if (distance_setting == "cosine") {
        distance = DistanceType::COSINE;
    } else if (distance_setting == "qgram") {
        distance = DistanceType::QGRAM;
    } else {
        distance = DistanceType::MINHASH;  // default
    }

    // 添加use_approx参数
    bool use_approx = true;  // 默认为true
    if (argc > 9 && argv[9] != nullptr) {
        std::string approx_setting = argv[9];
        if (approx_setting == "false") {
            use_approx = false;
        }
    }

    // Print parameters for verification
    std::cout << "\nUsing parameters:" << std::endl;
    std::cout << "  Compressor: " << compressor_setting << std::endl;
    std::cout << "  Window size: " << window_size << std::endl;
    std::cout << "  Log length: " << log_length << std::endl;
    std::cout << "  Threshold: " << threshold << std::endl;
    std::cout << "  Block size: " << block_size << std::endl;
    std::cout << "  Distance function: " << distance_setting << std::endl;
    std::cout << "  Use approximation: " << (use_approx ? "true" : "false") << std::endl;

    try {
        double time_cost = main_encoding_compress(
            input_path, 
            output_path, 
            window_size, 
            log_length, 
            threshold, 
            block_size,
            compressor,
            distance,
            use_approx
        );
        
        std::cout << "Compression completed in " << time_cost << " seconds." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
#endif // RECORD_COMPRESS