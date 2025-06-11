#ifndef RECORD_DECOMPRESS_HPP
#define RECORD_DECOMPRESS_HPP

#include "rle.hpp"
#include "utils.hpp"
#include "distance.hpp"
#include "bit_buffer.hpp"
#include "bit_packing.hpp"
#include "qgram_match.hpp"
#include "record_compress.hpp"
#include "variable_length_substitution.hpp"
#include "ts_2diff.hpp"
#include <chrono>
#include <deque>
#include <fstream>
#include <iostream>
#include <bitset>
#include <stdexcept>
#include <iomanip>

// Define macro for decoding statistics output
#ifndef DECODING_STATS
#define DECODING_STATS 0  // Set to 1 to enable statistics output
#endif

#define PRINT_STATS(x) if (DECODING_STATS) { std::cout << x << std::endl; }

// Forward declarations
void printRecord(const Record& record, int idx);
std::vector<Record> byteArrayDecoding(BitInBuffer& stream);
double main_decoding_decompress(const std::string& input_path, const std::string& output_path);

// Struct definitions for internal use
struct Method0Fields {
    int another_line;
    int begin;
    int operation_size;
    std::vector<int> position_list;
    std::vector<int> d_length;
    std::vector<int> i_length;
    std::vector<std::string> sub_string;
};

struct Method1Fields {
    int another_line;
    std::string sub_string;
};

#endif // RECORD_DECOMPRESS_HPP
