#include "bit_buffer.hpp"
#include "ts_2diff.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <iostream>
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <chrono>
#include <sys/stat.h>
#include <bitset>

constexpr int block_size = 64;

// Encode a single block of integers into the bit stream
size_t encode_block(BitOutBuffer& stream, const std::vector<int>& data) {
    if (data.empty()) return 0;

    if (data.size() == 1) {
        stream.encode(data[0], 32);
        stream.encode(0, 32);
        stream.encode(0, 12);
        return (32 + 32 + 12 + 7) / 8;  // 转换为字节数
    }

    std::vector<int> delta(data.size() - 1);
    for (size_t i = 1; i < data.size(); ++i) {
        delta[i - 1] = data[i] - data[i - 1];
    }
    int min_delta = *std::min_element(delta.begin(), delta.end());
    int max_delta = *std::max_element(delta.begin(), delta.end());
    int bit_len = std::max(max_delta - min_delta, 1);
    int bit_width = 0;
    while ((1 << bit_width) <= bit_len && bit_width < 32) ++bit_width;
    if (bit_width == 0) bit_width = 1;  // 确保至少使用1位

    // std::cout << "[ENCODE BLOCK] first_value=" << data[0]
    //           << ", min_delta=" << min_delta
    //           << ", delta_length=" << delta.size()
    //           << ", bit_width=" << bit_width << std::endl;

    stream.encode(data[0], 32);
    stream.encode(min_delta, 32);
    stream.encode((int)delta.size(), 12);
    stream.encode(bit_width, 8);
    for (size_t i = 0; i < delta.size(); ++i) {
        int enc = delta[i] - min_delta;
        stream.encode(enc, bit_width);
    }
    stream.pack();  // 确保所有位都被写入
    return (32 + 32 + 12 + 8 + bit_width * delta.size() + 7) / 8;  // 转换为字节数
}

// Encode a full integer vector to file using ts2diff algorithm
size_t ts2diff_encode(const std::vector<int>& data, BitOutBuffer& stream) {
    size_t bcnt = data.size() / block_size;
    size_t realBcnt = (data.size() + block_size - 1) / block_size;
    stream.encode((int)realBcnt, 32);
    size_t total_bytes = 4;  // 初始32位转换为字节
    for (size_t i = 0; i < bcnt; ++i) {
        total_bytes += encode_block(stream, std::vector<int>(data.begin() + i * block_size, data.begin() + (i + 1) * block_size));
    }
    if (bcnt * block_size < data.size()) {
        total_bytes += encode_block(stream, std::vector<int>(data.begin() + bcnt * block_size, data.end()));
    }
    return total_bytes;
}

// Decode a single block of integers from the bit stream
std::vector<int> decode_block(BitInBuffer& stream) {
    stream.align();  // 使用公开方法确保位对齐
    std::vector<int> result;
    int first_value = stream.decode(32);
    int min_delta = stream.decode(32);
    int delta_length = stream.decode(12);
    if (delta_length == 0) {
        result.push_back(first_value);
        return result;
    }
    int bit_width = stream.decode(8);
    if (bit_width < 1 || bit_width > 32) {
        throw std::runtime_error("Invalid bit width: " + std::to_string(bit_width));
    }

    // std::cout << "[DECODE BLOCK] first_value=" << first_value
    //           << ", min_delta=" << min_delta
    //           << ", delta_length=" << delta_length
    //           << ", bit_width=" << bit_width << std::endl;

    std::vector<int> delta(delta_length);
    for (int i = 0; i < delta_length; ++i) {
        int d = stream.decode(bit_width);
        delta[i] = d + min_delta;
    }
    // 累加还原
    result.reserve(delta_length + 1);
    int acc = first_value;
    result.push_back(acc);
    for (int i = 0; i < delta_length; ++i) {
        acc += delta[i];
        result.push_back(acc);
    }

    return result;
}

// Decode a file using ts2diff algorithm into a vector of integers
std::vector<int> ts2diff_decode(BitInBuffer& stream) {
    std::vector<int> result;
    int realBcnt = stream.decode(32);
    for (int i = 0; i < realBcnt; ++i) {
        std::vector<int> block = decode_block(stream);
        result.insert(result.end(), block.begin(), block.end());
    }
    stream.align();  // 确保解码后对齐到字节边界
    return result;
}


// 获取文件字节数
size_t get_file_size(const std::string& filename) {
    struct stat st;
    if (stat(filename.c_str(), &st) == 0)
        return st.st_size;
    return 0;
}

#ifdef TS2DIFF_TEST
// Test function to verify if two vectors are equal
bool verify_result(const std::vector<int>& original, const std::vector<int>& decoded) {
    if (original.size() != decoded.size()) {
        std::cout << "Size mismatch: original=" << original.size() 
                  << ", decoded=" << decoded.size() << std::endl;
        return false;
    }
    for (size_t i = 0; i < original.size(); ++i) {
        if (original[i] != decoded[i]) {
            return false;
        }
    }
    return true;
}

// Print vector contents for debugging
void print_vector(const std::string& name, const std::vector<int>& vec) {
    std::cout << name << ": [";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i < vec.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

// Test function for performance measurement
void test_performance(const std::vector<int>& data, const std::string& test_name) {
    // 编码
    BitOutBuffer outbuf;
    auto start = std::chrono::high_resolution_clock::now();
    size_t encoded_size = ts2diff_encode(data, outbuf);
    auto encode_end = std::chrono::high_resolution_clock::now();

    // 写入文件
    std::string filename = test_name + ".bin";
    outbuf.write(filename);

    // 用inbuffer读取文件
    BitInBuffer inbuf;
    inbuf.read(filename);

    // 解码
    auto decode_start = std::chrono::high_resolution_clock::now();
    std::vector<int> decoded = ts2diff_decode(inbuf);
    auto decode_end = std::chrono::high_resolution_clock::now();

    double encode_time = std::chrono::duration<double>(encode_end - start).count();
    double decode_time = std::chrono::duration<double>(decode_end - decode_start).count();

    size_t original_size = data.size() * 8;
    size_t compressed_size = get_file_size(filename);
    double ratio = compressed_size == 0 ? 0.0 : (double)original_size / compressed_size;

    std::cout << "\nPerformance test: " << test_name << std::endl;
    std::cout << "Data size: " << data.size() << " elements" << std::endl;
    std::cout << "Encoded size: " << encoded_size << " bytes" << std::endl;
    std::cout << "Encode time: " << encode_time << " seconds" << std::endl;
    std::cout << "Decode time: " << decode_time << " seconds" << std::endl;
    std::cout << "Original size: " << original_size << " bytes (int64 per value)" << std::endl;
    std::cout << "Compressed file size: " << compressed_size << " bytes" << std::endl;
    std::cout << "Compression ratio: " << ratio << std::endl;
    std::cout << "Test " << (verify_result(data, decoded) ? "PASSED" : "FAILED") << std::endl;
}

// 测试混合编码
void test_mixed_encoding() {
    std::cout << "\nTesting mixed encoding..." << std::endl;
    
    // 准备测试数据
    std::vector<int> data = {1, 3, 5, 7, 9, 11, 13, 15};
    int extra_num1 = 42;
    int extra_num2 = 100;
    
    // 编码
    BitOutBuffer outbuf;
    size_t encoded_size = ts2diff_encode(data, outbuf);
    std::cout << "ts2diff encoded size: " << encoded_size << " bytes" << std::endl;
    
    outbuf.encode(extra_num1, 32);
    outbuf.encode(extra_num2, 32);
    
    // 写入文件
    std::string filename = "mixed_test.bin";
    outbuf.write(filename);
    
    // 读取和解码
    BitInBuffer inbuf;
    inbuf.read(filename);
    
    // 先解码ts2diff数据
    std::cout << "Decoding ts2diff data..." << std::endl;
    std::vector<int> decoded = ts2diff_decode(inbuf);
    std::cout << "ts2diff decode completed, decoded size: " << decoded.size() << std::endl;
    
    // 再解码额外的两个数
    std::cout << "Decoding extra numbers..." << std::endl;
    int decoded_num1 = inbuf.decode(32);
    int decoded_num2 = inbuf.decode(32);
    
    // 验证结果
    std::cout << "Original data: ";
    print_vector("data", data);
    std::cout << "Extra numbers: " << extra_num1 << ", " << extra_num2 << std::endl;
    
    std::cout << "Decoded data: ";
    print_vector("decoded", decoded);
    std::cout << "Decoded extra numbers: " << decoded_num1 << ", " << decoded_num2 << std::endl;
    
    bool data_match = verify_result(data, decoded);
    bool extra_match = (extra_num1 == decoded_num1) && (extra_num2 == decoded_num2);
    
    std::cout << "Data match: " << (data_match ? "PASSED" : "FAILED") << std::endl;
    std::cout << "Extra numbers match: " << (extra_match ? "PASSED" : "FAILED") << std::endl;
    std::cout << "Total test: " << (data_match && extra_match ? "PASSED" : "FAILED") << std::endl;
}

int main() {
    // Test case 1: Simple ascending sequence
    std::vector<int> test1 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    test_performance(test1, "test1");

    // Test case 2: Mixed positive and negative numbers
    std::vector<int> test2 = {-100, -50, 0, 50, 100, -200, 200, -300, 300};
    test_performance(test2, "test2");

    // Test case 3: Large sequence with repeated patterns
    std::vector<int> test3;
    for (int i = 0; i < 1000; ++i) test3.push_back(i % 100);
    test_performance(test3, "test3");

    // Test case 4: Single value
    std::vector<int> test4 = {42};
    test_performance(test4, "test4");

    // Test case 5: Alternating sequence
    std::vector<int> test5;
    for (int i = 0; i < 100; ++i) test5.push_back(i % 2 == 0 ? 100 : -100);
    test_performance(test5, "test5");

    // Test case 6: Random sequence
    std::vector<int> test6;
    srand(42);
    for (int i = 0; i < 1000; ++i) test6.push_back(rand() % 1000 - 500);
    test_performance(test6, "test6");

    // Test case 7: Performance test with large dataset
    std::vector<int> test7;
    for (int i = 0; i < 100000; ++i) test7.push_back(i * 2);
    test_performance(test7, "test7");

    // Test case 8: Performance test with complex pattern
    std::vector<int> test8;
    for (int i = 0; i < 300000; ++i) test8.push_back(i % 3 == 0 ? -1 : 1);
    test_performance(test8, "test8");

    // Test case 9: Mixed encoding test
    test_mixed_encoding();

    return 0;
}
#endif // TS2DIFF_TEST
