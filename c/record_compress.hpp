#ifndef RECORD_COMPRESS_HPP
#define RECORD_COMPRESS_HPP

#include <string>
#include <vector>
#include "bit_buffer.hpp"
#include "distance.hpp"

// Custom data structure to replace DataFrame
struct Record {
    int method;
    int another_line;
    int begin;
    int operation_size;
    std::vector<int> position_list;
    std::vector<int> d_length;
    std::vector<int> i_length;
    std::vector<std::string> sub_string;
};

// Function declarations
void byteArrayEncoding(const std::vector<Record>& records, 
                      const std::string& output_path, 
                      CompressorType compressor = CompressorType::NONE);

double main_encoding_compress_exact(const std::string& input_path, 
                            const std::string& output_path, 
                            int window_size = 8,
                            int log_length = 256,
                            double threshold = 0.06,
                            int block_size = 32768,
                            CompressorType compressor = CompressorType::NONE,
                            DistanceType distance = DistanceType::MINHASH);

double main_encoding_compress_approx(const std::string& input_path, 
                            const std::string& output_path, 
                            int window_size = 8,
                            int log_length = 256,
                            double threshold = 0.06,
                            int block_size = 32768,
                            CompressorType compressor = CompressorType::NONE,
                            DistanceType distance = DistanceType::MINHASH);

#endif // RECORD_COMPRESS_HPP 