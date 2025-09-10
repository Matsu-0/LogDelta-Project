#ifndef RECORD_COMPRESS_HPP
#define RECORD_COMPRESS_HPP

#include <string>
#include <vector>
#include "bit_buffer.hpp"
#include "distance.hpp"

// Global default parameters
namespace DefaultParams {
    const int WINDOW_SIZE = 8;
    const double THRESHOLD = 0.06;
    const int BLOCK_SIZE = 32768000;
    const int Q = 3;  // Default Q-gram size
    const CompressorType COMPRESSOR = CompressorType::LZMA;
    const DistanceType DISTANCE = DistanceType::MINHASH;
    const bool USE_APPROX = true;
}

// Custom data structure for storing compression records
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
                      CompressorType compressor = DefaultParams::COMPRESSOR);

double main_encoding_compress(const std::string& input_path, 
                            const std::string& output_path, 
                            int window_size = DefaultParams::WINDOW_SIZE,
                            // int log_length = 0,
                            double threshold = DefaultParams::THRESHOLD,
                            int block_size = DefaultParams::BLOCK_SIZE,
                            CompressorType compressor = DefaultParams::COMPRESSOR,
                            DistanceType distance = DefaultParams::DISTANCE,
                            bool use_approx = DefaultParams::USE_APPROX);

#endif // RECORD_COMPRESS_HPP 