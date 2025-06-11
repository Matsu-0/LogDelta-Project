#include "rle.hpp"
#include "utils.hpp"
#include <algorithm>
#include <iostream>
#include <vector>
#include <cassert>

// Calculate the number of bits in binary representation
// Return at least 2 for numbers less than 2
int getBinaryLength(int num) {
    int len = 0;
    while (num > 0) {
        len++;
        num /= 2;
    }
    return (len < 2) ? 2 : len;
}

// Encode a single number into binary string
std::string encodeNumber(int num) {
    int n = getBinaryLength(num);
    std::string result;
    
    // Add (n-2) '1's
    result.append(n-2, '1');
    result += '0';
    
    // Convert num to binary
    std::string binary;
    if (num == 1) {
        binary = "01";
    } else {
        while (num > 0) {
            binary = (char)('0' + (num % 2)) + binary;
            num /= 2;
        }
    }
    
    result += binary;
    return result;
}

// Decode a single number from binary string
int decodeNumber(const std::string& binary, size_t& pos) {
    int countOnes = 0;
    
    // Count leading ones
    while (pos < binary.length() && binary[pos] == '1') {
        countOnes++;
        pos++;
    }
    
    // Skip the '0'
    if (pos >= binary.length()) {
        throw std::runtime_error("Invalid binary string format: missing '0' after ones");
    }
    pos++;
    
    // Read the binary number
    int value = 0;
    int bitsToRead = countOnes + 2;
    if (pos + bitsToRead > binary.length()) {
        throw std::runtime_error("Invalid binary string format: not enough bits for number");
    }
    
    for (int i = 0; i < bitsToRead; i++) {
        value = (value << 1) | (binary[pos + i] - '0');
    }
    pos += bitsToRead;
    
    return value;
}

RLEEncoded rleEncode(const std::vector<int>& arr) {
    if (arr.empty()) return {{}, 0};
    
    std::string result;
    result += ('0' + arr[0]);  // Store initial value
    
    // Calculate intervals between switches
    std::vector<int> intervals;
    int count = 1;
    
    for (size_t i = 1; i < arr.size(); i++) {
        if (arr[i] != arr[i-1]) {
            intervals.push_back(count);
            count = 1;
        } else {
            count++;
        }
    }
    intervals.push_back(count);
    
    // Encode each interval
    for (int interval : intervals) {
        result += encodeNumber(interval);
    }
    
    // Pad the result to ensure it's a multiple of 8 bits
    while (result.length() % 8 != 0) {
        result += '0';
    }
    
    // Convert the binary string to bytes using the existing function
    return {stringToBytes(result), intervals.size()};
}

std::vector<int> rleDecode(const std::vector<unsigned char>& encoded, size_t interval_count) {
    // Convert bytes back to binary string using the existing function
    std::string binaryString = bytesToString(encoded);
    if (binaryString.empty() || interval_count == 0) return {};
    
    int currentValue = binaryString[0] - '0';
    size_t pos = 1;
    std::vector<int> result;
    size_t decoded_intervals = 0;
    try {
        while (pos < binaryString.length() && decoded_intervals < interval_count) {
        int interval = decodeNumber(binaryString, pos);
            if (interval <= 0) {
                throw std::runtime_error("Invalid interval value: " + std::to_string(interval));
            }
        result.insert(result.end(), interval, currentValue);
        currentValue = 1 - currentValue;  // Toggle between 0 and 1
            decoded_intervals++;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during RLE decoding: " << e.what() << std::endl;
        throw;
    }
    return result;
}

// Test cases for RLE encoding and decoding
void testRLE() {
    std::cout << "\n=== RLE Test Cases ===" << std::endl;
    
    // Test case 1: Simple alternating pattern
    {
        std::vector<int> input = {0, 1, 0, 1, 0, 1, 0, 1};
        std::cout << "\nTest Case 1 - Alternating pattern:" << std::endl;
        std::cout << "Input: ";
        for (auto v : input) std::cout << v << " ";
        std::cout << std::endl;
        
        auto encoded = rleEncode(input);
        std::cout << "Encoded bytes: ";
        for (auto b : encoded.bytes) std::cout << std::hex << static_cast<int>(b) << " ";
        std::cout << std::dec << std::endl;
        std::cout << "Interval count: " << encoded.interval_count << std::endl;
        
        auto decoded = rleDecode(encoded.bytes, encoded.interval_count);
        std::cout << "Decoded: ";
        for (auto v : decoded) std::cout << v << " ";
        std::cout << std::endl;
        
        assert(decoded == input);
    }
    
    // Test case 2: Long runs
    {
        std::vector<int> input = {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0};
        std::cout << "\nTest Case 2 - Long runs:" << std::endl;
        std::cout << "Input: ";
        for (auto v : input) std::cout << v << " ";
        std::cout << std::endl;
        
        auto encoded = rleEncode(input);
        std::cout << "Encoded bytes: ";
        for (auto b : encoded.bytes) std::cout << std::hex << static_cast<int>(b) << " ";
        std::cout << std::dec << std::endl;
        std::cout << "Interval count: " << encoded.interval_count << std::endl;
        
        auto decoded = rleDecode(encoded.bytes, encoded.interval_count);
        std::cout << "Decoded: ";
        for (auto v : decoded) std::cout << v << " ";
        std::cout << std::endl;
        
        assert(decoded == input);
    }
    
    // Test case 3: Single values
    {
        std::vector<int> input = {0, 1, 0, 1, 0};
        std::cout << "\nTest Case 3 - Single values:" << std::endl;
        std::cout << "Input: ";
        for (auto v : input) std::cout << v << " ";
        std::cout << std::endl;
        
        auto encoded = rleEncode(input);
        std::cout << "Encoded bytes: ";
        for (auto b : encoded.bytes) std::cout << std::hex << static_cast<int>(b) << " ";
        std::cout << std::dec << std::endl;
        std::cout << "Interval count: " << encoded.interval_count << std::endl;
        
        auto decoded = rleDecode(encoded.bytes, encoded.interval_count);
        std::cout << "Decoded: ";
        for (auto v : decoded) std::cout << v << " ";
        std::cout << std::endl;
        
        assert(decoded == input);
    }
    
    // Test case 4: Mixed patterns
    {
        std::vector<int> input = {0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0};
        std::cout << "\nTest Case 4 - Mixed patterns:" << std::endl;
        std::cout << "Input: ";
        for (auto v : input) std::cout << v << " ";
        std::cout << std::endl;
        
        auto encoded = rleEncode(input);
        std::cout << "Encoded bytes: ";
        for (auto b : encoded.bytes) std::cout << std::hex << static_cast<int>(b) << " ";
        std::cout << std::dec << std::endl;
        std::cout << "Interval count: " << encoded.interval_count << std::endl;
        
        auto decoded = rleDecode(encoded.bytes, encoded.interval_count);
        std::cout << "Decoded: ";
        for (auto v : decoded) std::cout << v << " ";
        std::cout << std::endl;
        
        assert(decoded == input);
    }
    
    // Test case 5: Empty vector
    {
        std::vector<int> input;
        std::cout << "\nTest Case 5 - Empty vector:" << std::endl;
        std::cout << "Input: (empty)" << std::endl;
    
        auto encoded = rleEncode(input);
        std::cout << "Encoded bytes: ";
        for (auto b : encoded.bytes) std::cout << std::hex << static_cast<int>(b) << " ";
        std::cout << std::dec << std::endl;
        std::cout << "Interval count: " << encoded.interval_count << std::endl;
        
        auto decoded = rleDecode(encoded.bytes, encoded.interval_count);
        std::cout << "Decoded: (empty)" << std::endl;
        
        assert(decoded == input);
    }
    
    // Test case 6: All zeros
    {
        std::vector<int> input(10, 0);
        std::cout << "\nTest Case 6 - All zeros:" << std::endl;
        std::cout << "Input: ";
        for (auto v : input) std::cout << v << " ";
        std::cout << std::endl;
        
        auto encoded = rleEncode(input);
    std::cout << "Encoded bytes: ";
        for (auto b : encoded.bytes) std::cout << std::hex << static_cast<int>(b) << " ";
        std::cout << std::dec << std::endl;
        std::cout << "Interval count: " << encoded.interval_count << std::endl;
        
        auto decoded = rleDecode(encoded.bytes, encoded.interval_count);
        std::cout << "Decoded: ";
        for (auto v : decoded) std::cout << v << " ";
        std::cout << std::endl;
        
        assert(decoded == input);
    }
    
    // Test case 7: All ones
    {
        std::vector<int> input(10, 1);
        std::cout << "\nTest Case 7 - All ones:" << std::endl;
        std::cout << "Input: ";
        for (auto v : input) std::cout << v << " ";
        std::cout << std::endl;
        
        auto encoded = rleEncode(input);
        std::cout << "Encoded bytes: ";
        for (auto b : encoded.bytes) std::cout << std::hex << static_cast<int>(b) << " ";
        std::cout << std::dec << std::endl;
        std::cout << "Interval count: " << encoded.interval_count << std::endl;
    
        auto decoded = rleDecode(encoded.bytes, encoded.interval_count);
        std::cout << "Decoded: ";
        for (auto v : decoded) std::cout << v << " ";
        std::cout << std::endl;
        
        assert(decoded == input);
    }
    
    // Test case 8: Large alternating pattern (50000 elements)
    {
        std::vector<int> input;
        input.reserve(50000);
        for (int i = 0; i < 50000; i++) {
            input.push_back(i % 2);
        }
        std::cout << "\nTest Case 8 - Large alternating pattern (50000 elements):" << std::endl;
        std::cout << "Input size: " << input.size() << std::endl;
        std::cout << "First 10 elements: ";
        for (int i = 0; i < 10; i++) {
            std::cout << input[i] << " ";
        }
        std::cout << "..." << std::endl;
        
        auto encoded = rleEncode(input);
        std::cout << "Encoded bytes: ";
        for (size_t i = 0; i < std::min(20UL, encoded.bytes.size()); i++) {
            std::cout << std::hex << static_cast<int>(encoded.bytes[i]) << " ";
        }
        std::cout << "..." << std::dec << std::endl;
        std::cout << "Total encoded bytes: " << encoded.bytes.size() << std::endl;
        std::cout << "Interval count: " << encoded.interval_count << std::endl;
        
        auto decoded = rleDecode(encoded.bytes, encoded.interval_count);
        std::cout << "Decoded size: " << decoded.size() << std::endl;
        std::cout << "First 10 decoded elements: ";
        for (int i = 0; i < 10; i++) {
            std::cout << decoded[i] << " ";
        }
        std::cout << "..." << std::endl;
        
        assert(decoded == input);
        std::cout << "Compression ratio: " << (double)(encoded.bytes.size() * 8) / input.size() << " bits per element" << std::endl;
    }
    
    std::cout << "\nAll RLE tests passed!" << std::endl;
    }
    
#ifdef RLE_TEST
int main() {
    testRLE();
    return 0;
}
#endif