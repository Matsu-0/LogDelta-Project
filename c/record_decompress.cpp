#include "record_compress.hpp"
#include "bit_buffer.hpp"
#include "bit_packing.hpp"
#include "rle.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <bitset>
#include <deque>
#include "variable_length_substitution.hpp"

std::vector<Record> byteArrayDecoding(const std::string& input_path, CompressorType compressor) {
    BitInBuffer stream;
    if (!stream.read(input_path, compressor)) {
        throw std::runtime_error("Failed to read input file: " + input_path);
    }
    std::vector<Record> records;
    // 读取method0和method1的数量
    int records0_size = stream.decode(16);
    int records1_size = stream.decode(16);
    int total_records = records0_size + records1_size;

    // 解码method_list
    int method_length = stream.decode(16);
    std::vector<unsigned char> rle_bytes(method_length);
    for (int i = 0; i < method_length; ++i) {
        rle_bytes[i] = stream.decode(8);
    }
    std::vector<int> method_list = rleDecode(rle_bytes, total_records);

    // 解码another_line_list
    int line_length = stream.decode(16);
    std::vector<unsigned char> rle_bytes2(line_length);
    for (int i = 0; i < line_length; ++i) {
        rle_bytes2[i] = stream.decode(8);
    }
    std::vector<int> another_line_list = rleDecode(rle_bytes2, total_records);

    // 解码begin
    int begin_length = stream.decode(16);
    std::vector<int> begins;
    if (begin_length > 0) {
        std::vector<unsigned char> packed_bytes(begin_length);
        for (int i = 0; i < begin_length; ++i) packed_bytes[i] = stream.decode(8);
        begins = bit_packing_decode(packed_bytes, records0_size);
    }

    // 解码operation_size
    int op_size_length = stream.decode(16);
    std::vector<int> operation_sizes;
    if (op_size_length > 0) {
        std::vector<unsigned char> packed_bytes(op_size_length);
        for (int i = 0; i < op_size_length; ++i) packed_bytes[i] = stream.decode(8);
        operation_sizes = bit_packing_decode(packed_bytes, records0_size);
    }

    // 解码length_list
    int length_blocks = 1; // 这里需与编码端保持一致
    std::vector<int> length_list;
    for (int b = 0; b < length_blocks; ++b) {
        int leng = stream.decode(16);
        if (leng > 0) {
            std::vector<unsigned char> packed_bytes(leng);
            for (int i = 0; i < leng; ++i) packed_bytes[i] = stream.decode(8);
            std::vector<int> block = bit_packing_decode(packed_bytes, records0_size * 2); // 近似
            length_list.insert(length_list.end(), block.begin(), block.end());
        }
    }

    // 解码positions
    int pos_blocks = stream.decode(16);
    std::vector<int> p_begin_list, p_delta_list;
    for (int b = 0; b < pos_blocks; ++b) {
        int leng = stream.decode(16);
        if (leng > 0) {
            std::vector<unsigned char> packed_bytes(leng);
            for (int i = 0; i < leng; ++i) packed_bytes[i] = stream.decode(8);
            std::vector<int> block = bit_packing_decode(packed_bytes, records0_size); // 近似
            p_begin_list.insert(p_begin_list.end(), block.begin(), block.end());
        }
    }
    int delta_blocks = 1; // 这里需与编码端保持一致
    for (int b = 0; b < delta_blocks; ++b) {
        int leng = stream.decode(16);
        if (leng > 0) {
            std::vector<unsigned char> packed_bytes(leng);
            for (int i = 0; i < leng; ++i) packed_bytes[i] = stream.decode(8);
            std::vector<int> block = bit_packing_decode(packed_bytes, records0_size); // 近似
            p_delta_list.insert(p_delta_list.end(), block.begin(), block.end());
        }
    }

    // 解码sub_string
    // 先统计所有sub_string的总长度（需与编码端一致，这里假设每个sub_string为1字节，实际需根据原始数据格式处理）
    std::vector<std::string> all_sub_strings;
    std::vector<size_t> sub_string_lengths;
    size_t sub_string_total = 0;
    size_t idx0 = 0, idx1 = 0, len_idx = 0;
    for (int i = 0; i < total_records; ++i) {
        if (method_list[i] == 0) {
            int op_size = operation_sizes[idx0];
            for (int j = 0; j < op_size; ++j) {
                // 假设每个sub_string长度为1，实际应有长度信息
                sub_string_lengths.push_back(1);
                sub_string_total += 1;
            }
            idx0++;
        } else {
            sub_string_lengths.push_back(1);
            sub_string_total += 1;
        }
    }
    // 读取所有sub_string
    for (size_t i = 0; i < sub_string_total; ++i) {
        char c = static_cast<char>(stream.decode(8));
        all_sub_strings.push_back(std::string(1, c));
    }

    // 组装Record
    records.resize(total_records);
    idx0 = 0; // 0型record索引
    idx1 = 0; // 1型record索引
    size_t pos_begin = 0, pos_delta = 0, sub_idx = 0, len_ptr = 0;
    for (int i = 0; i < total_records; ++i) {
        records[i].method = method_list[i];
        records[i].another_line = another_line_list[i];
        if (records[i].method == 0) {
            // begin
            records[i].begin = begins[idx0];
            // operation_size
            int op_size = operation_sizes[idx0];
            records[i].operation_size = op_size;
            // position_list
            std::vector<int> pos_list;
            if (op_size > 0) {
                pos_list.push_back(p_begin_list[pos_begin++]);
                for (int j = 1; j < op_size; ++j) {
                    pos_list.push_back(pos_list.back() + p_delta_list[pos_delta++]);
                }
            }
            records[i].position_list = pos_list;
            // d_length/i_length
            std::vector<int> dlen, ilen;
            for (int j = 0; j < op_size; ++j) dlen.push_back(length_list[len_ptr++]);
            for (int j = 0; j < op_size; ++j) ilen.push_back(length_list[len_ptr++]);
            records[i].d_length = dlen;
            records[i].i_length = ilen;
            // sub_string
            for (int j = 0; j < op_size; ++j) {
                records[i].sub_string.push_back(all_sub_strings[sub_idx++]);
            }
            idx0++;
        } else {
            // 1型record只有sub_string
            records[i].sub_string.push_back(all_sub_strings[sub_idx++]);
            idx1++;
        }
    }
    return records;
}

// 还原原始行
std::vector<std::string> restore_lines(const std::vector<Record>& records, int window_size) {
    std::vector<std::string> result;
    std::deque<std::string> q;
    for (const auto& rec : records) {
        if (rec.method == 1) {
            std::string line = rec.sub_string[0];
            result.push_back(line);
            if ((int)q.size() < window_size) q.push_back(line);
            else { q.pop_front(); q.push_back(line); }
        } else {
            // 用OperationItem组装op_list
            std::vector<OperationItem> op_list;
            for (int i = 0; i < rec.operation_size; ++i) {
                op_list.emplace_back(rec.position_list[i], rec.d_length[i], rec.i_length[i], rec.sub_string[i]);
            }
            std::string base = q[rec.begin];
            std::string line = recoverSubstitutionString(op_list, base);
            result.push_back(line);
            if ((int)q.size() < window_size) q.push_back(line);
            else { q.pop_front(); q.push_back(line); }
        }
    }
    return result;
}

void main_decoding_decompress(const std::string& input_path, int window_size = 8) {

    BitInBuffer stream;
    if (!stream.read(input_path)) {
        throw std::runtime_error("Failed to read input file: " + input_path);
    }

    std::vector<Record> records = byteArrayDecoding(input_path);
    std::vector<std::string> lines = restore_lines(records, window_size);
    std::cout << "Restored " << lines.size() << " lines." << std::endl;
    for (size_t i = 0; i < std::min(lines.size(), size_t(10)); ++i) {
        std::cout << "Line " << i << ": " << lines[i] << std::endl;
    }
}

#ifdef RECORD_DECOMPRESS
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_path> [window_size]" << std::endl;
        return 1;
    }
    std::string input_path = argv[1];
    int window_size = 8;
    if (argc > 2) window_size = std::stoi(argv[2]);
    try {
        main_decoding_decompress(input_path, window_size);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
#endif // RECORD_DECOMPRESS
