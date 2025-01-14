#ifndef RECORD_COMPRESS_HPP
#define RECORD_COMPRESS_HPP

#include <string>
#include <vector>
#include "bit_buffer.hpp"

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

double main_encoding_compress_approx(const std::string& input_path, 
                            const std::string& output_path, 
                            int window_size = 8,
                            int log_length = 256,
                            double threshold = 0.035,
                            int block_size = 32768,
                            CompressorType compressor = CompressorType::NONE);

// Add global variables for statistics
extern size_t total_method_size;
extern size_t total_line_size;
extern size_t total_begin_size;
extern size_t total_operation_size;
extern size_t total_length_size;
extern size_t total_position_size;
extern size_t total_string_size;

extern size_t total_method_compressed;
extern size_t total_line_compressed;
extern size_t total_begin_compressed;
extern size_t total_operation_compressed;
extern size_t total_length_compressed;
extern size_t total_position_compressed;
extern size_t total_string_compressed;

#endif // RECORD_COMPRESS_HPP 