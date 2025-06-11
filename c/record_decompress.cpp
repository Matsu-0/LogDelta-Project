#include "rle.hpp"
#include "utils.hpp"
#include "distance.hpp"
#include "bit_buffer.hpp"
#include "bit_packing.hpp"
#include "qgram_match.hpp"
#include "record_compress.hpp"
#include "variable_length_substitution.hpp"
#include "ts_2diff.hpp"
#include "record_decompress.hpp"
#include <chrono>
#include <deque>
#include <fstream>
#include <iostream>
#include <bitset>
#include <stdexcept>
#include <iomanip>


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

std::vector<Record> byteArrayDecoding(BitInBuffer& stream) {
    // Read record counts
    int records0_size = stream.decode(32);
    int records1_size = stream.decode(32);
    PRINT_STATS("Records0 size: " << records0_size << ", Records1 size: " << records1_size);

    // If both sizes are 0, we've reached the end of a block
    if (records0_size == 0 && records1_size == 0) {
        return std::vector<Record>();
    }

    // Decode method using RLE
    int method_length = stream.decode(16);
    std::string method_string;
    for (int i = 0; i < method_length; i++) {
        int bf = stream.decode(8);
        method_string += std::bitset<8>(bf).to_string();
    }
    std::vector<unsigned char> method_bytes = stringToBytes(method_string);
    size_t method_interval_count = stream.decode(32);
    std::vector<int> method_list = rleDecode(method_bytes, method_interval_count);
    PRINT_STATS("Decoded method list size: " << method_list.size());

    // // Decode another_line using RLE
    // int line_length = stream.decode(16);
    // std::string line_string;
    // for (int i = 0; i < line_length; i++) {
    //     int bf = stream.decode(8);
    //     line_string += std::bitset<8>(bf).to_string();
    // }
    // std::vector<unsigned char> line_bytes = stringToBytes(line_string);
    // size_t line_interval_count = stream.decode(32);
    // std::vector<int> another_line_list = rleDecode(line_bytes, line_interval_count);
    // PRINT_STATS("Decoded Line list size: " << another_line_list.size());

    // Create records array
    std::vector<Record> records;
    records.reserve(records0_size + records1_size);

    // Decode begins using bit packing
    std::vector<int> begins;
    if (records0_size > 0) {
        int begin_length = stream.decode(16);
        std::string bit_packing_string;
        for (int i = 0; i < begin_length; i++) {
            int bf = stream.decode(8);
            bit_packing_string += std::bitset<8>(bf).to_string();
        }
        std::vector<unsigned char> begins_bytes = stringToBytes(bit_packing_string);
        begins = bit_packing_decode(begins_bytes, records0_size);
        PRINT_STATS("Decoded begins size: " << begins.size());
    }

    // Decode operation_sizes using bit packing
    std::vector<int> operation_sizes;
    if (records0_size > 0) {
        int operation_length = stream.decode(16);
        std::string bit_packing_string;
        for (int i = 0; i < operation_length; i++) {
            int bf = stream.decode(8);
            bit_packing_string += std::bitset<8>(bf).to_string();
        }
        std::vector<unsigned char> sizes_bytes = stringToBytes(bit_packing_string);
        operation_sizes = bit_packing_decode(sizes_bytes, records0_size);
        PRINT_STATS("Decoded operation size: " << operation_sizes.size());
    }

    // Create temporary array to store method 0 data
    std::vector<int> length_list;
    std::vector<int> p_begin_list;
    std::vector<int> p_delta_list;
    int record0_idx = 0;  // Used to track current processing record of method 0

    // Decode lengths
    if (records0_size > 0) {
        int total_lengths = 0;
        for (int i = 0; i < records0_size; i++) {
            total_lengths += operation_sizes[i] * 2;
        }
        length_list = ts2diff_decode(stream);
        PRINT_STATS("Decoded lengths size: " << length_list.size());

        // Validate if length_list size is correct
        if (length_list.size() != static_cast<size_t>(total_lengths)) {
            throw std::runtime_error("Length list size mismatch: expected " + 
                                   std::to_string(total_lengths) + ", got " + 
                                   std::to_string(length_list.size()));
        }
    }

    // Decode positions
    if (records0_size > 0) {
        p_begin_list = ts2diff_decode(stream);
        PRINT_STATS("Total begin positions: " << p_begin_list.size());
        p_delta_list = ts2diff_decode(stream);
        PRINT_STATS("Total delta positions: " << p_delta_list.size());
    }

    // First, decode all fields of method0 and method1
    struct Method0Fields {
        // int another_line;
        int begin;
        int operation_size;
        std::vector<int> position_list;
        std::vector<int> d_length;
        std::vector<int> i_length;
        std::vector<std::string> sub_string;
    };
    struct Method1Fields {
        // int another_line;
        std::string sub_string;
    };
    std::vector<Method0Fields> method0_vec;
    std::vector<Method1Fields> method1_vec;

    // Decode method0
    record0_idx = 0;
    int length_idx = 0, begin_idx = 0, delta_idx = 0;
    for (size_t i = 0; i < method_list.size(); i++) {
        if (method_list[i] == 0) {
            Method0Fields m0;
            // m0.another_line = another_line_list[i];
            m0.begin = begins[record0_idx];
            m0.operation_size = operation_sizes[record0_idx];
            
            // Decode positions based on operation_size
            if (m0.operation_size > 0) {
                int pos = p_begin_list[begin_idx++];
                m0.position_list.push_back(pos);
                for (int j = 1; j < m0.operation_size; j++) {
                    pos += p_delta_list[delta_idx++];
                    m0.position_list.push_back(pos);
                }
                
                // Decode lengths based on operation_size
                for (int j = 0; j < m0.operation_size; j++) {
                    m0.d_length.push_back(length_list[length_idx++]);
                }
                for (int j = 0; j < m0.operation_size; j++) {
                    m0.i_length.push_back(length_list[length_idx++]);
                }
                
                // Decode substrings based on operation_size
                for (int j = 0; j < m0.operation_size; j++) {
                    std::string substr;
                    int len = m0.i_length[j];
                    for (int k = 0; k < len; k++) {
                        char byte = static_cast<char>(stream.decode(8));
                        substr += byte;
                    }
                    m0.sub_string.push_back(substr);
                }
            }
            
            // Validate array sizes
            if (m0.position_list.size() != static_cast<size_t>(m0.operation_size) ||
                m0.d_length.size() != static_cast<size_t>(m0.operation_size) ||
                m0.i_length.size() != static_cast<size_t>(m0.operation_size) ||
                m0.sub_string.size() != static_cast<size_t>(m0.operation_size)) {
                throw std::runtime_error("Array size mismatch in record " + std::to_string(i) + 
                    ": operation_size=" + std::to_string(m0.operation_size) +
                    ", position_list=" + std::to_string(m0.position_list.size()) +
                    ", d_length=" + std::to_string(m0.d_length.size()) +
                    ", i_length=" + std::to_string(m0.i_length.size()) +
                    ", sub_string=" + std::to_string(m0.sub_string.size()));
            }
            
            method0_vec.push_back(m0);
            record0_idx++;
        }
    }
    // Decode method1
    for (size_t i = 0; i < method_list.size(); i++) {
        if (method_list[i] == 1) {
            Method1Fields m1;
            // m1.another_line = another_line_list[i];
            std::string substr;
            while (true) {
                char byte = static_cast<char>(stream.decode(8));
                if (byte == '\n') break;
                substr += byte;
            }
            m1.sub_string = substr;
            method1_vec.push_back(m1);
        }
    }
    // Reconstruct records based on method_list order
    size_t idx0 = 0, idx1 = 0;
    for (size_t i = 0; i < method_list.size(); ++i) {
        if (method_list[i] == 0) {
            const auto& m0 = method0_vec[idx0++];
            Record record;
            record.method = 0;
            // record.another_line = m0.another_line;
            record.begin = m0.begin;
            record.operation_size = m0.operation_size;
            record.position_list = m0.position_list;
            record.d_length = m0.d_length;
            record.i_length = m0.i_length;
            record.sub_string = m0.sub_string;
            records.push_back(record);
        } else {
            const auto& m1 = method1_vec[idx1++];
            Record record;
            record.method = 1;
            // record.another_line = m1.another_line;
            record.begin = 0;
            record.operation_size = 0;
            record.position_list.clear();
            record.d_length.clear();
            record.i_length.clear();
            record.sub_string = {m1.sub_string};
            records.push_back(record);
        }
    }

    return records;
}

double main_decoding_decompress(const std::string& input_path, 
                               const std::string& output_path) {
    auto total_start_time = std::chrono::high_resolution_clock::now();

    // Time statistics for each part
    double read_time = 0;
    double decoding_time = 0;
    double recovery_time = 0;
    double write_time = 0;

    // Read encoding head
    auto read_start = std::chrono::high_resolution_clock::now();
    BitInBuffer stream;
    stream.read(input_path);
    
    int window_size = stream.decode(16);
    if (window_size == 0) {
        std::cerr << "Error: Window size is 0, this is invalid." << std::endl;
        throw std::runtime_error("Invalid window size: 0");
    }
    
    
    uint8_t param_byte = stream.decode(8);
    bool use_approx = (param_byte >> 7) & 0x1;
    auto read_end = std::chrono::high_resolution_clock::now();
    read_time = std::chrono::duration<double>(read_end - read_start).count();

    std::deque<std::string> q;
    std::ofstream output(output_path);
    if (!output.is_open()) {
        throw std::runtime_error("Failed to open output file: " + output_path);
    }

    try {
        // Process blocks until end of file
        auto last_recovery_end = std::chrono::high_resolution_clock::now();
        while (true) {
            try {
                // Decode records for current block
                auto decode_start = std::chrono::high_resolution_clock::now();
                std::vector<Record> records = byteArrayDecoding(stream);
                auto decode_end = std::chrono::high_resolution_clock::now();
                decoding_time += std::chrono::duration<double>(decode_end - decode_start).count();

                // If no records were decoded, we've reached the end
                if (records.empty()) {
                    break;
                }
                
                // Process each record in the block
                int record_count = 0;
                auto recovery_start = std::chrono::high_resolution_clock::now();
                for (const auto& record : records) {
                    std::string line;
                    if (record.method == 0) {
                        // Check if we have enough records in the window
                        if (record.begin >= static_cast<int>(q.size())) {
                            std::cerr << "Error at record " << record_count << ":" << std::endl;
                            std::cerr << "  Method: " << record.method << std::endl;
                            std::cerr << "  Begin: " << record.begin << std::endl;
                            std::cerr << "  Window size: " << q.size() << std::endl;
                            std::cerr << "  Operation size: " << record.operation_size << std::endl;
                            std::cerr << "  Position list size: " << record.position_list.size() << std::endl;
                            std::cerr << "  D length size: " << record.d_length.size() << std::endl;
                            std::cerr << "  I length size: " << record.i_length.size() << std::endl;
                            std::cerr << "  Sub string size: " << record.sub_string.size() << std::endl;
                            throw std::runtime_error("Invalid reference index: " + std::to_string(record.begin) + 
                                                   ", window size: " + std::to_string(q.size()));
                        }
                        // Reconstruct line from reference
                        line = q[record.begin];
                        if (use_approx) {
                            // Use Q-gram matching recovery
                            std::vector<OperationItem> ops;
                            for (size_t i = 0; i < record.position_list.size(); i++) {
                                ops.emplace_back(record.position_list[i], 
                                              record.d_length[i],
                                              record.i_length[i],
                                              record.sub_string[i]);
                            }
                            line = recoverQgramString(ops, line);
                        } else {
                            // Use exact substitution recovery
                            std::vector<OperationItem> ops;
                            for (size_t i = 0; i < record.position_list.size(); i++) {
                                ops.emplace_back(record.position_list[i], 
                                              record.d_length[i],
                                              record.i_length[i],
                                              record.sub_string[i]);
                            }
                            line = recoverSubstitutionString(ops, line);
                        }
                    } else {
                        // Direct output for method 1
                        line = record.sub_string[0];
                    }

                    record_count++;

                    // // Write the line to output
                    // if (record.another_line == 1) {
                    //     // If another_line is 1, remove the trailing newline
                    //     if (!line.empty() && line.back() == '\n') {
                    //         line.pop_back();
                    //     }
                    //     output << line;  // Don't add newline for another_line=1
                    // } else {
                    //     output << line << std::endl;  // Add newline for another_line=0
                    // }

                    output << line << std::endl;

                    // Update sliding window
                    if (q.size() < static_cast<size_t>(window_size)) {
                        q.push_back(line);
                    } else {
                        q.pop_front();
                        q.push_back(line);
                    }
                }
                last_recovery_end = std::chrono::high_resolution_clock::now();
                recovery_time += std::chrono::duration<double>(last_recovery_end - recovery_start).count();

            } catch (const std::runtime_error& e) {
                // If we get an end of file error, we're done
                if (std::string(e.what()).find("Attempting to read past end of buffer") != std::string::npos) {
                    break;
                }
                // Otherwise, rethrow the error
                throw;
            }
        }

        output.close();
        auto write_end = std::chrono::high_resolution_clock::now();
        write_time = std::chrono::duration<double>(write_end - last_recovery_end).count();

    } catch (const std::exception& e) {
        std::cerr << "Error occurred during processing:" << std::endl;
        std::cerr << e.what() << std::endl;
        throw std::runtime_error("Error during decompression: " + std::string(e.what()));
    }

    auto total_end_time = std::chrono::high_resolution_clock::now();
    double total_time = std::chrono::duration<double>(total_end_time - total_start_time).count();

    // Print time statistics
    std::cout << "Time statistics:" << std::endl;
    std::cout << "  Read time: " << read_time << " seconds" << std::endl;
    std::cout << "  Decoding time: " << decoding_time << " seconds" << std::endl;
    std::cout << "  Recovery time: " << recovery_time << " seconds" << std::endl;
    std::cout << "  Write time: " << write_time << " seconds" << std::endl;
    std::cout << "  Total time: " << total_time << " seconds" << std::endl;

    return total_time;
}

#ifdef RECORD_DECOMPRESS
int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input_path> <output_path>" << std::endl;
        return 1;
    }

    std::string input_path = argv[1];
    std::string output_path = argv[2];

    try {
        double time_cost = main_decoding_decompress(
            input_path, 
            output_path
        );
        
        // Use time_cost to avoid unused variable warning
        (void)time_cost;  // Explicitly ignore the return value
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
#endif // RECORD_DECOMPRESS
